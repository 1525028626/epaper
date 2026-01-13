#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <lvgl.h>

#include "gui_port/gui_port.h"
#include "system/SysEvent.h"
#include "system/PageManager.h"
#include "system/System.h"

// 引入首发 App
#include "app/app_home.h"

// === 全局变量 ===
QueueHandle_t g_gui_queue = NULL;
QueueHandle_t g_worker_queue = NULL;
TaskHandle_t hGuiTask = NULL;

// ==========================================
// Task 1: GUI 线程 (Core 1)
// 负责 UI 渲染、LVGL 定时器、事件分发
// ==========================================
void Task_GUI(void *pvParameters) {
    sys_event_t event;

    // 启动第一个 App
    PageManager::loadApp(new App_Home());

    while (1) {
        // 1. LVGL 核心
        lv_timer_handler();

        // 2. 处理后台消息 (Worker -> GUI)
        if (xQueueReceive(g_gui_queue, &event, 0) == pdTRUE) {
            PageManager::handleEvent(&event);
        }

        // 3. 触摸唤醒 (配合 gui_port 的触摸扫描)
        // 没触摸睡 10ms，有触摸立马醒
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));
    }
}

// ==========================================
// Task 2: Worker 线程 (Core 0)
// 负责后台业务逻辑、网络请求、系统管理
// ==========================================
void Task_Worker(void *pvParameters) {
    sys_event_t cmd;

    // 可以在这里初始化 WiFi (不卡 UI)
    // WiFi.begin("SSID", "PASS");

    while (1) {
        // 1. 让当前 App 跑后台逻辑 (如果有)
        PageManager::loop();

        // 2. 处理 GUI 指令 (GUI -> Worker)
        // 等待 5ms，避免死循环空转
        if (xQueueReceive(g_worker_queue, &cmd, 5) == pdTRUE) {
            switch (cmd.type) {
                case CMD_FETCH_WEATHER: {
                    Serial.println("[Worker] Fetching Weather...");
                    // 模拟网络耗时
                    vTaskDelay(pdMS_TO_TICKS(1500)); 
                    
                    // 拿到数据，发回给 UI
                    // 这里模拟 25 度
                    System::sendToUI(EVT_DATA_WEATHER, 25);
                    break;
                }
                
                case CMD_SYSTEM_REBOOT:
                    ESP.restart();
                    break;
                    
                default: break;
            }
        }
        
        // 3. 周期性任务 (例如每分钟更新时间)
        static uint32_t last_tick = 0;
        if (millis() - last_tick > 60000) {
            last_tick = millis();
            // System::sendToUI(EVT_TIME_UPDATED);
        }
    }
}

// ==========================================
// Setup
// 系统入口：初始化硬件、创建队列、启动任务
// ==========================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== System Booting ===");
    
    // 检查 PSRAM
    if (psramFound()) {
        Serial.printf("PSRAM: %d KB available\n", ESP.getPsramSize() / 1024);
    } else {
        Serial.println("Warning: PSRAM not found! Check build_flags.");
    }

    // 1. 创建双向通信队列
    g_gui_queue = xQueueCreate(20, sizeof(sys_event_t));
    g_worker_queue = xQueueCreate(20, sizeof(sys_event_t));

    // 2. 初始化底层硬件 (屏幕、触摸、LVGL)
    gui_port_init();

    // 3. 创建双核任务
    // GUI 跑 Core 1 (高优)
    xTaskCreatePinnedToCore(Task_GUI, "GUI", 8192, NULL, 2, &hGuiTask, 1);
    
    // Worker 跑 Core 0 (低优)
    xTaskCreatePinnedToCore(Task_Worker, "Worker", 8192, NULL, 1, NULL, 0);
    
    Serial.println(">>> System Started.");
}

void loop() {
    vTaskDelete(NULL); // 销毁 Arduino 默认任务
}