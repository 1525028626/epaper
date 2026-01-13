#include "App_Home.h"
#include "../ui/ui.h" // SquareLine 生成的 UI

/**
 * @brief App 启动回调
 * 初始化 UI 资源，设置初始画面，并发起必要的数据请求
 */
void App_Home::onStart() {
    Serial.println("[App] Home: Start");

    // 1. 初始化 UI (按需加载)
    // 务必确保 ui_HomePage 是 NULL 才能 init，防止重复创建
    if (!ui_HomePage) {
        ui_HomePage_screen_init();
    }
    
    // 2. 切换画面 (带动画)
    lv_scr_load_anim(ui_HomePage, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);

    // 3. 业务逻辑: 请求刷新天气
    // 刚进主页，让后台去查一下天气
    System::sendToWorker(CMD_FETCH_WEATHER);
}

/**
 * @brief App 停止回调
 * 释放 UI 资源，防止内存泄漏
 */
void App_Home::onStop() {
    Serial.println("[App] Home: Stop");
    
    // 1. 销毁 UI (释放 RAM)
    if (ui_HomePage) {
        lv_obj_del(ui_HomePage);
        ui_HomePage = NULL; // 必须置空!
    }
}

/**
 * @brief 事件处理回调
 * 处理来自 Worker 线程的数据更新或系统事件
 */
void App_Home::onEvent(sys_event_t* event) {
    switch (event->type) {
        case EVT_DATA_WEATHER: {
            int temp = event->arg;
            Serial.printf("[App] Home: Weather Update -> %d C\n", temp);
            
            // 假设 UI 上有一个 ui_LabelTemp，更新它
            // if (ui_LabelTemp) lv_label_set_text_fmt(ui_LabelTemp, "%d C", temp);
            break;
        }
        default: break;
    }
}