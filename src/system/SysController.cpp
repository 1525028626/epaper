#include "SysController.h"
#include "PageManager.h"
#include "common/board_pins.h"
#include "common/Log.h" // 引入日志系统
#include "bsp/bsp_touch.h" // 引入触摸驱动以重新初始化
#include "gui_port/gui_port.h" // 引入 GUI 接口以控制任务挂起

/**
 * @file SysController.cpp
 * @brief 系统控制器实现文件
 * @details 包含了系统初始化、主循环、事件分发及电源管理的具体实现。
 */

// 静态成员变量初始化
uint32_t SysController::last_activity_time = 0;

/**
 * @brief 初始化系统控制器
 */
void SysController::setup() {
    // 记录启动时间作为初始活动时间
    last_activity_time = millis();

    // 配置触摸屏中断引脚为输入上拉模式
    // 这一步至关重要，因为 FT6336 在触摸时会将 INT 引脚拉低
    pinMode(PIN_TOUCH_INT, INPUT_PULLUP);

    // 启用 GPIO 唤醒功能
    // 当 GPIO 13 (PIN_TOUCH_INT) 检测到低电平时，将唤醒处于 Light Sleep 的 ESP32
    gpio_wakeup_enable((gpio_num_t)PIN_TOUCH_INT, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    LOG_I("[Sys] Controller Setup Complete");
}

/**
 * @brief 系统主循环
 * @note 此函数在 Task_Worker (Core 0) 中无限循环调用
 */
void SysController::loop() {
    // 1. 执行当前 App 的后台逻辑
    // 例如：如果当前是天气 App，它可能需要定时检查更新状态
    PageManager::loop();

    // 2. 自动休眠检查
    // 如果超过 30 秒 (30000 ms) 没有活动，则进入浅睡眠
    if (shouldSleep(30000)) {
        LOG_I("[Pwr] Idle timeout, preparing for Light Sleep...");
        LOG_FLUSH(); // 确保串口数据发送完毕
        
        // 1. GUI 准备：挂起触摸任务，防止 I2C 冲突，关闭屏幕电源
        gui_enter_sleep();

        // 再次 Flush 确保 gui_enter_sleep 的日志已输出
        LOG_FLUSH();
        delay(10); // 给串口最后一点时间

        // 2. 进入 Light Sleep 模式
        // - CPU 暂停，RAM 保持
        // - 仅保留 GPIO 唤醒
        esp_light_sleep_start();
        
        // --- 系统被唤醒 (WAKE UP POINT) ---
        
        // 3. 硬件恢复
        delay(100); // 等待时钟稳定
        LOG_INIT(115200); // 恢复串口
        
        // 4. 关键：强制重置 I2C 总线并重新初始化触摸芯片
        // 必须在恢复触摸任务之前完成
        bsp_touch_init();
        
        // 5. GUI 恢复：恢复触摸任务，唤醒屏幕
        gui_exit_sleep();
        
        LOG_I("[Pwr] Woke up!");
        // 唤醒后立即更新活动时间，避免立刻再次休眠
        updateActivity(); 
    }
}

/**
 * @brief 处理 Worker 线程接收到的事件
 * @param event 接收到的事件对象
 */
void SysController::handleEvent(sys_event_t *event) {
    switch (event->type) {
        case CMD_FETCH_WEATHER:
            LOG_I("[Sys] Fetching Weather...");
            // 模拟网络请求耗时 (1.5秒)
            // 在实际项目中，这里会调用 HTTP Client 获取 JSON 数据
            vTaskDelay(pdMS_TO_TICKS(1500));
            
            // 模拟获取成功，发送 EVT_DATA_WEATHER 事件回 GUI
            // 参数 25 表示温度
            sendToUI(EVT_DATA_WEATHER, 25);
            break;

        case CMD_SYSTEM_REBOOT:
            LOG_I("[Sys] Rebooting...");
            delay(100);
            ESP.restart(); // 软重启 ESP32
            break;
            
        default: 
            // 未知事件，忽略
            break;
    }
}

/**
 * @brief 发送事件到 Worker 队列
 */
void SysController::sendToWorker(SysEventType type, int32_t arg, void* payload) {
    sys_event_t evt = {type, payload, arg};
    // xQueueSend 是 FreeRTOS 线程安全的队列发送函数
    // 0 表示如果不成功立即返回，不阻塞
    if(g_worker_queue) xQueueSend(g_worker_queue, &evt, 0);
}

/**
 * @brief 发送事件到 GUI 队列
 */
void SysController::sendToUI(SysEventType type, int32_t arg, void* payload) {
    sys_event_t evt = {type, payload, arg};
    if(g_gui_queue) xQueueSend(g_gui_queue, &evt, 0);
}

/**
 * @brief 更新最后活动时间戳
 */
void SysController::updateActivity() {
    last_activity_time = millis();
}

/**
 * @brief 判断是否超时应休眠
 * @param timeout_ms 超时时间
 * @return true 应该休眠
 */
bool SysController::shouldSleep(uint32_t timeout_ms) {
    uint32_t now = millis();
    
    // 处理 millis() 溢出回绕的情况 (约 49 天发生一次)
    if (now < last_activity_time) {
        last_activity_time = now;
        return false;
    }
    
    // 检查时间差
    return (now - last_activity_time > timeout_ms);
}
