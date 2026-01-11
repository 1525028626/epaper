#include <Arduino.h>
#include <lvgl.h>
#include "gui_port/gui_port.h"
#include "system/PageManager.h"
#include "system/AppBase.h"
#include "ui/ui.h" // 引入 SquareLine 生成的 UI 头文件

// =========================================================
// 全局变量
// =========================================================

// 定义全局 GUI 任务句柄
// 必须定义它，因为 gui_port.cpp 中使用了 extern TaskHandle_t hGuiTask;
// 触摸扫描任务会通过这个句柄唤醒 GUI 线程
TaskHandle_t hGuiTask = NULL;

// =========================================================
// App: Main (SquareLine UI)
// =========================================================
class App_Main : public AppBase {
public:
    void onStart() override {
        Serial.println("App_Main: Starting UI...");
        // 初始化 SquareLine 生成的界面
        ui_init();
    }

    void onStop() override {
        Serial.println("App_Main: Stopping...");
        // 清理 ui.c 中创建的所有全局屏幕对象
        // 必须手动删除，否则下次加载时会重复创建导致内存泄漏
        if (ui_HomePage) lv_obj_del(ui_HomePage);
        if (ui_CalendarPage) lv_obj_del(ui_CalendarPage);
        if (ui_WeatherPage) lv_obj_del(ui_WeatherPage);
        if (ui_SettingPage) lv_obj_del(ui_SettingPage);
        if (ui_AppPage) lv_obj_del(ui_AppPage);

        ui_HomePage = NULL;
        ui_CalendarPage = NULL;
        ui_WeatherPage = NULL;
        ui_SettingPage = NULL;
        ui_AppPage = NULL;
    }
};

// =========================================================
// GUI 线程 (运行 LVGL 核心逻辑)
// =========================================================
void Task_GUI(void *pvParameters) {
    Serial.println("GUI Task Started.");

    // 加载主页面 App
    PageManager::loadApp(new App_Main());

    while (1) {
        // 1. 处理 LVGL 定时器 (UI 刷新、动画、事件分发)
        //    这是 LVGL 的心脏，必须定期调用
        lv_timer_handler();

        // 2. 智能延时 (配合 gui_port.cpp 中的触摸唤醒)
        //    - 如果有触摸事件，Task_Touch_Poller 会调用 xTaskNotifyGive(hGuiTask)
        //      此时 ulTaskNotifyTake 会立即返回，让 UI 瞬间响应。
        //    - 如果没有事件，这里最多等待 10ms，保证 lv_timer_handler 至少每 10ms 运行一次
        //      (用于处理动画或长按等定时事件)
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10)); 
    }
}

// =========================================================
// Arduino Setup
// =========================================================
void setup() {
    Serial.begin(115200);
    
    Serial.println("\n=== System Booting ===");

    gui_port_init();

    // 2. 创建 GUI 主任务
    //    优先级设为 2 (低于触摸扫描任务 3，高于 IDLE)
    //    分配 8KB 栈空间 (LVGL 需要较大栈，如果崩溃请尝试增大)
    //    运行在 Core 1 (与 Arduino loop 同核，但独立任务)
    xTaskCreatePinnedToCore(
        Task_GUI,   
        "GUI_Task", 
        8192,       
        NULL,       
        2,          
        &hGuiTask,  
        1           
    );
}

// =========================================================
// Arduino Loop
// =========================================================
void loop() {
    // 主循环可以处理非 GUI 的后台逻辑 (例如 WiFi 重连、传感器读取)
    // 或者直接调用 PageManager 的后台循环
    PageManager::loop();
    
    // 简单的延时，避免看门狗触发
    vTaskDelay(pdMS_TO_TICKS(100));
}
