/**
 * @file main.cpp
 * @brief 系统主入口文件
 * @details 负责系统启动、硬件自检、双核任务创建和消息队列初始化。
 *          采用双核架构：
 *          - Core 1: GUI 线程 (LVGL 渲染、事件分发)
 *          - Core 0: Worker 线程 (后台业务、网络请求、电源管理)
 */
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <lvgl.h>

#include "common/Log.h" // 引入日志系统
#include "gui_port/gui_port.h"
#include "system/SysEvent.h"
#include "system/PageManager.h"
#include "system/SysController.h" // SysController

// 引入首发 App
#include "app/app_home.h"

// === 全局变量 ===
QueueHandle_t g_gui_queue = NULL;     ///< GUI 线程消息队列 (接收来自 Worker 的消息)
QueueHandle_t g_worker_queue = NULL;  ///< Worker 线程消息队列 (接收来自 GUI 的消息)
TaskHandle_t hGuiTask = NULL;         ///< GUI 任务句柄 (用于触摸中断唤醒)

/* ==================================================================
 * Task 1: GUI 线程 (运行在 Core 1)
 * 职责：
 * 1. 运行 LVGL 核心定时器 (lv_timer_handler)
 * 2. 接收并处理来自 Worker 的业务事件
 * 3. 响应触摸输入
 * ================================================================== */
void Task_GUI(void *pvParameters) {
    sys_event_t event;

    // 启动第一个 App
    PageManager::loadApp(new App_Home());
    
    // 初始化活动计时
    SysController::updateActivity();

    while (1) {
        // 1. LVGL 核心
        // 处理绘制、输入设备读取、动画等
        lv_timer_handler();

        // 2. 处理后台消息 (Worker -> GUI)
        // 阻塞时间设为 0，因为 GUI 需要高频刷新，不能卡在这里
        if (xQueueReceive(g_gui_queue, &event, 0) == pdTRUE) {
            PageManager::handleEvent(&event);
            SysController::updateActivity(); // 收到消息视为活动
        }

        // 3. 触摸唤醒 (配合 gui_port 的触摸扫描)
        // 没触摸睡 10ms，有触摸立马醒，实现动态帧率
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));
    }
}

/* ==================================================================
 * Task 2: Worker 线程 (运行在 Core 0)
 * 职责：
 * 1. 运行后台业务逻辑 (WiFi, HTTP, Sensor)
 * 2. 处理来自 GUI 的控制指令
 * 3. 监控系统状态 (如自动休眠)
 * ================================================================== */
void Task_Worker(void *pvParameters) {
    // Controller Setup (初始化系统服务)
    SysController::setup();

    sys_event_t cmd;

    while (1) {
        // 1. Controller Loop (包含 App::loop, 自动休眠检测等)
        SysController::loop();

        // 2. 处理 GUI 指令 (GUI -> Worker)
        // 允许短暂阻塞，节省 CPU
        if (xQueueReceive(g_worker_queue, &cmd, 5) == pdTRUE) {
            SysController::handleEvent(&cmd);
        }
    }
}

/* ==================================================================
 * Setup
 * 系统启动流程：
 * 1. 串口初始化
 * 2. PSRAM 自检
 * 3. 创建通信队列
 * 4. 初始化底层硬件 (屏幕、触摸)
 * 5. 创建双核任务
 * ================================================================== */
void setup() {
    LOG_INIT(115200);
    delay(500);
    LOG_I("\n=== System Booting ===");
    
    // 检查 PSRAM (对于大屏幕和 LVGL 来说至关重要)
    if (psramFound()) {
        LOG_I("PSRAM: %d KB available", ESP.getPsramSize() / 1024);
    } else {
        LOG_E("Warning: PSRAM not found! Check build_flags.");
    }
    LOG_I("Flash: %d KB", ESP.getFlashChipSize() / 1024);

    // 1. 创建双向通信队列
    g_gui_queue = xQueueCreate(20, sizeof(sys_event_t));
    g_worker_queue = xQueueCreate(20, sizeof(sys_event_t));

    // 2. 初始化底层硬件 (屏幕、触摸、LVGL)
    gui_port_init();

    // 3. 创建双核任务
    // GUI 跑 Core 1 (Arduino loop 默认跑在 Core 1，这里显式创建以获得更好控制)
    xTaskCreatePinnedToCore(Task_GUI, "GUI", 8192, NULL, 2, &hGuiTask, 1);
    
    // Worker 跑 Core 0 (独立于 GUI，互不卡顿)
    xTaskCreatePinnedToCore(Task_Worker, "Worker", 8192, NULL, 1, NULL, 0);
    
    LOG_I(">>> System Started.");
}

void loop() {
    // Arduino 默认 loop 任务任务已无用，自杀以释放资源
    vTaskDelete(NULL); 
}
