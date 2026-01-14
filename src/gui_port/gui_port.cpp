#include "gui_port.h"
#include <lvgl.h>
#include <Arduino.h>
#include "common/Log.h" // 引入日志系统
#include "bsp/bsp_epd.h"
#include "bsp/bsp_touch.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "system/SysController.h" // 用于活动计时

#define BLACK 0x00
#define WHITE 0xFF

#define PAINT_BUF_SIZE (EPD_WIDTH * EPD_HEIGHT / 8)

// 【优化】利用 PSRAM，直接分配全屏缓冲区，提升渲染性能
// 176 * 264 = 46,464 像素
#define LVGL_BUF_SIZE (EPD_WIDTH * EPD_HEIGHT)

// LVGL 双缓冲区指针
static lv_color_t *buf_1 = NULL;
static lv_color_t *buf_2 = NULL;

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

// 【优化】改为指针，后续在 PSRAM 中动态分配
uint8_t *Paint_Image = NULL;
uint8_t *Shadow_Image = NULL;

static uint8_t WidthByte = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);

TaskHandle_t hEPDTask = NULL;
// 引用 main.cpp 里的 GUI 任务句柄
extern TaskHandle_t hGuiTask; 
// 触摸任务句柄，用于休眠时挂起
TaskHandle_t hTouchTask = NULL;

volatile bool is_epd_busy = false;

// === 全局触摸缓存 (用于任务间通信) ===
// 避免 GUI 线程再次读取 I2C，直接拿结果
volatile bool g_touch_pressed = false;
volatile int16_t g_touch_x = 0;
volatile int16_t g_touch_y = 0;

// 内部绘图辅助函数声明
void Paint_SetPixel(uint16_t x, uint16_t y, uint16_t color);
void Paint_Clear(uint16_t color);

/* ==================================================================
 * 1. 独立触摸扫描任务 (高优先级侦察兵)
 * 解决 "没人唤醒 GUI" 的死锁问题
 * 负责高频读取 I2C 触摸屏，并更新全局状态，必要时唤醒 GUI 线程
 * ================================================================== */
void Task_Touch_Poller(void *pvParameters) {
    touch_point_t tp;
    bool last_pressed = false;
    
    while(1) {
        bool current_pressed = false;

        // 1. 屏蔽刷新期间的触摸干扰 (关键修复)
        // 墨水屏刷新时的高压驱动会产生电磁干扰，导致触摸屏误报上一次的坐标(幽灵触控)
        // 仅在屏幕空闲时读取触摸
        if (bsp_touch_read(&tp)) {
            // 适配横屏模式 (LVGL hor_res = EPD_HEIGHT, ver_res = EPD_WIDTH)
            // 交换 X/Y 轴，并处理镜像
            // 物理 Y (0..264) -> 逻辑 X (0..264)
            // 物理 X (0..176) -> 逻辑 Y (0..176)
            g_touch_x = EPD_HEIGHT - 1 - tp.y;
            g_touch_y = tp.x;
            
            // 防止坐标越界
            if (g_touch_x < 0) g_touch_x = 0;
            if (g_touch_x >= EPD_HEIGHT) g_touch_x = EPD_HEIGHT - 1;
            if (g_touch_y < 0) g_touch_y = 0;
            if (g_touch_y >= EPD_WIDTH) g_touch_y = EPD_WIDTH - 1;

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
 * 由 LVGL 内部定时器调用
 * ================================================================== */
void my_touch_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    if (g_touch_pressed) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = g_touch_x;
        data->point.y = g_touch_y;
        LOG_D("LVGL Touch: x=%d, y=%d", data->point.x, data->point.y);
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/* ==================================================================
 * 3. 显示刷新 (保持不变)
 * LVGL 渲染完成后的回调，将像素数据转换为墨水屏格式并触发刷新
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
    
    // 如果是最后一块数据，触发物理刷新
    if (lv_disp_flush_is_last(disp_drv)) {
        // 【优化】移除 is_epd_busy 检查，强制刷新
        // 之前如果上一帧正在刷，会直接丢弃当前帧，导致页面切换后屏幕不更新
        // 虽然有撕裂风险，但对于 EPD 来说，显示最新内容更重要
        // if (!is_epd_busy) {
            memcpy(Shadow_Image, Paint_Image, PAINT_BUF_SIZE);
            if (hEPDTask != NULL) xTaskNotifyGive(hEPDTask);
        // }
    }
    lv_disp_flush_ready(disp_drv);
}

/* ==================================================================
 * 4. 后台刷屏任务
 * 接收刷新信号，执行耗时的 SPI 刷屏操作，避免阻塞 GUI 线程
 * ================================================================== */
void Task_EPD_Refresh(void *pvParameters) {
    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        is_epd_busy = true;
// 刷屏
        bsp_epd_display_full(Shadow_Image); 
        
        is_epd_busy = false;
        
        // 刷屏也算活动
        SysController::updateActivity();
    }
}

/* ==================================================================
 * 初始化
 * 配置硬件、分配内存、创建任务、注册 LVGL 驱动
 * ================================================================== */
void gui_port_init(void) {
    // 1. 硬件初始化
    bsp_epd_init();
    bsp_touch_init();

    lv_init();
    
    // 使用 MALLOC_CAP_SPIRAM 分配到 PSRAM
    // 分配 LVGL 全屏双缓冲 (约 180KB)
    buf_1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf_2 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

    // 分配墨水屏显存 (约 12KB)
    Paint_Image = (uint8_t *)heap_caps_malloc(PAINT_BUF_SIZE, MALLOC_CAP_SPIRAM);
    Shadow_Image = (uint8_t *)heap_caps_malloc(PAINT_BUF_SIZE, MALLOC_CAP_SPIRAM);

    // 简单的空指针检查 (防止 PSRAM 未启用导致崩溃)
    if (!buf_1 || !Paint_Image) {
        LOG_E("ERROR: Failed to allocate memory in PSRAM!");
    }

    Paint_Clear(WHITE); 
    memcpy(Shadow_Image, Paint_Image, PAINT_BUF_SIZE);
    
    // 刷一次白屏 (注释掉以加快启动速度，且避免首帧被忽略的问题)
    // bsp_epd_clear(WHITE); 

    // 创建后台刷屏任务 (Core 0)
    xTaskCreatePinnedToCore(Task_EPD_Refresh, "EPD_Ref", 4096, NULL, 1, &hEPDTask, 0);

    // 5. 创建独立的高频触摸扫描任务 (Core 1, 优先级高于 LVGL)
    xTaskCreatePinnedToCore(Task_Touch_Poller, "TouchPoller", 4096, NULL, 5, &hTouchTask, 1);

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

    LOG_I("GUI Port Initialized.");
}

/**
 * @brief 进入休眠前的准备
 */
void gui_enter_sleep(void) {
    LOG_I("[GUI] Preparing for sleep...");
    LOG_FLUSH(); // 确保日志输出
    
    // 1. 挂起触摸任务，防止在休眠过程中访问 I2C
    // 注意：强制挂起可能导致 I2C 事务中断，但由于唤醒后会重置 I2C，所以是可接受的
    if (hTouchTask != NULL) {
        vTaskSuspend(hTouchTask);
    }
    
    // 等待一小会儿确保任务已暂停
    delay(10);
    
    // 2. 让电子纸进入深睡模式 (降低功耗)
    bsp_epd_sleep();
    
    LOG_I("[GUI] Sleep ready.");
    LOG_FLUSH();
}

/**
 * @brief 唤醒后的恢复
 */
void gui_exit_sleep(void) {
    LOG_I("[GUI] Waking up...");
    
    // 1. 恢复触摸任务
    if (hTouchTask != NULL) {
        vTaskResume(hTouchTask);
    }
    
    // 2. 电子纸唤醒 (通常需要重新 init)
    // 注意: bsp_epd_init 可能会比较慢，且屏幕内容在 Deep Sleep 中通常会保持
    // 如果 bsp_epd_init 会清屏，则需要重绘。如果只是电气唤醒，则不需要。
    // 这里假设需要重新初始化才能再次发送命令
    bsp_epd_init();
    
    LOG_I("[GUI] Wake up done.");
}

/**
 * @brief 设置显存中的像素点颜色
 * 将 LVGL 的颜色映射到墨水屏的 1bit 缓冲区 (Paint_Image)
 */
void Paint_SetPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= EPD_WIDTH || y >= EPD_HEIGHT) return; 
    uint32_t Addr = x / 8 + y * WidthByte;
    uint8_t Rdata = Paint_Image[Addr];
    if(color == BLACK) Paint_Image[Addr] = Rdata & ~(0x80 >> (x % 8));
    else               Paint_Image[Addr] = Rdata | (0x80 >> (x % 8));
}

/**
 * @brief 清空显存
 */
void Paint_Clear(uint16_t color) {
    for (uint32_t i = 0; i < PAINT_BUF_SIZE; i++) {
        Paint_Image[i] = (color == WHITE) ? 0xFF : 0x00;
    }
}