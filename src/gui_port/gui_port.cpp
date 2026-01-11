#include "gui_port.h"
#include <lvgl.h>
#include <Arduino.h>
#include "bsp/bsp_epd.h"   
#include "bsp/bsp_touch.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define BLACK 0
#define WHITE 1

#define PAINT_BUF_SIZE (EPD_WIDTH * EPD_HEIGHT / 8)
// 减小 LVGL 缓冲区，节省内存给双核任务
#define LVGL_BUF_SIZE (EPD_WIDTH * EPD_HEIGHT / 10)

static lv_color_t *buf_1 = NULL;
static lv_color_t *buf_2 = NULL;
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

uint8_t Paint_Image[PAINT_BUF_SIZE];
uint8_t Shadow_Image[PAINT_BUF_SIZE];

static uint8_t WidthByte = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);

TaskHandle_t hEPDTask = NULL;
// 引用 main.cpp 里的 GUI 任务句柄
extern TaskHandle_t hGuiTask; 
volatile bool is_epd_busy = false;

// === 全局触摸缓存 (用于任务间通信) ===
// 避免 GUI 线程再次读取 I2C，直接拿结果
volatile bool g_touch_pressed = false;
volatile int16_t g_touch_x = 0;
volatile int16_t g_touch_y = 0;

void Paint_SetPixel(uint16_t x, uint16_t y, uint16_t color);
void Paint_Clear(uint16_t color);

/* ==================================================================
 * 1. 独立触摸扫描任务 (高优先级侦察兵)
 * 解决 "没人唤醒 GUI" 的死锁问题
 * ================================================================== */
void Task_Touch_Poller(void *pvParameters) {
    touch_point_t tp;
    bool last_pressed = false;
    
    while(1) {
        bool current_pressed = false;

        // 每 10ms 扫描一次 (非常快，不占资源)
        if (bsp_touch_read(&tp)) {
            // 1. 读到触摸，存入全局缓存
            //    (注意：这里直接做坐标转换)
            
            // 还原为交换轴逻辑 (因为你反馈原来的触摸方向是正确的)
            // 但修正 Y 轴的计算，防止溢出导致一直点击底部
                   
            g_touch_x = EPD_HEIGHT - 1 - tp.y;
            g_touch_y = tp.x;
            current_pressed = true;
        }
        
        g_touch_pressed = current_pressed;

        // 2. 【核心修复】猛踢 GUI 线程
        // 只要状态发生变化 (按下->抬起，或 抬起->按下) 或者 保持按下(拖动)
        // 都要唤醒 GUI 线程，否则 LVGL 接收不到 RELEASE 事件，点击无效
        if (current_pressed || (current_pressed != last_pressed)) {
            if (hGuiTask != NULL) xTaskNotifyGive(hGuiTask);
        }
        
        last_pressed = current_pressed;

        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

/* ==================================================================
 * 2. LVGL 输入回调
 * 现在它不需要读 I2C 了，直接读全局变量 (极速)
 * ================================================================== */
void my_touch_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    if (g_touch_pressed) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = g_touch_x;
        data->point.y = g_touch_y;
        Serial.printf("LVGL Touch: x=%d, y=%d\n", data->point.x, data->point.y);
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/* ==================================================================
 * 3. 显示刷新 (保持不变)
 * ================================================================== */
void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    
    for(int y_lv = area->y1; y_lv <= area->y2; y_lv++) {
        for(int x_lv = area->x1; x_lv <= area->x2; x_lv++) {
            uint16_t pixel_color = color_p[(x_lv - area->x1) + (y_lv - area->y1) * w].full;
            if(pixel_color==0xffff){
                Paint_SetPixel(y_lv, x_lv, WHITE);
            }else{
                Paint_SetPixel(y_lv, x_lv, BLACK);

            }
        }
    }
    
    if (lv_disp_flush_is_last(disp_drv)) {
        if (!is_epd_busy) {
            memcpy(Shadow_Image, Paint_Image, PAINT_BUF_SIZE);
            if (hEPDTask != NULL) xTaskNotifyGive(hEPDTask);
        }
    }
    lv_disp_flush_ready(disp_drv);
}

/* ==================================================================
 * 4. 后台刷屏任务
 * ================================================================== */
void Task_EPD_Refresh(void *pvParameters) {
    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        is_epd_busy = true;
        bsp_epd_display_full(Shadow_Image); 
        is_epd_busy = false;
    }
}

/* ==================================================================
 * 初始化
 * ================================================================== */
void gui_port_init(void) {
    // 1. 硬件初始化
    bsp_epd_init();
    bsp_touch_init();

    lv_init();
    
    buf_1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    buf_2 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);

    Paint_Clear(WHITE); 
    memcpy(Shadow_Image, Paint_Image, PAINT_BUF_SIZE);
    
    // 刷一次白屏
    bsp_epd_clear(WHITE); 

    // 创建后台刷屏任务 (Core 0)
    xTaskCreatePinnedToCore(Task_EPD_Refresh, "EPD_Ref", 4096, NULL, 1, &hEPDTask, 0);

    // 创建独立触摸扫描任务 (Core 1, 优先级 3 - 比 GUI 更高!)
    xTaskCreatePinnedToCore(Task_Touch_Poller, "Touch_Scan", 2048, NULL, 3, NULL, 1);

    lv_disp_draw_buf_init(&draw_buf, buf_1, buf_2, LVGL_BUF_SIZE);
    lv_disp_drv_init(&disp_drv);    

    disp_drv.hor_res = EPD_HEIGHT; 
    disp_drv.ver_res = EPD_WIDTH;  
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = disp_flush;
    disp_drv.full_refresh = 0; // 局部刷新

    lv_disp_drv_register(&disp_drv);
    
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);
}

void Paint_SetPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return; 
    uint32_t Addr = x / 8 + y * WidthByte;
    uint8_t Rdata = Paint_Image[Addr];
    if(color == BLACK) Paint_Image[Addr] = Rdata & ~(0x80 >> (x % 8));
    else               Paint_Image[Addr] = Rdata | (0x80 >> (x % 8));
}

void Paint_Clear(uint16_t color) {
    for (uint32_t i = 0; i < PAINT_BUF_SIZE; i++) {
        Paint_Image[i] = (color == WHITE) ? 0xFF : 0x00;
    }
}