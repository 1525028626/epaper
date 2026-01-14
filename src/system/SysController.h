#ifndef SYSTEM_H
#define SYSTEM_H

#include "SysEvent.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <Arduino.h>

/**
 * @file SysController.h
 * @brief 系统控制器核心头文件
 * @details 定义了系统控制器类，负责全局任务调度、跨线程通信和电源管理。
 */

// 引用 main.cpp 中的全局队列句柄
// g_gui_queue: 用于向 GUI 线程发送事件
extern QueueHandle_t g_gui_queue;
// g_worker_queue: 用于向 Worker 线程发送指令
extern QueueHandle_t g_worker_queue;

/**
 * @class SysController
 * @brief 系统控制器类 (静态单例模式)
 * 
 * @details 
 * 该类作为系统的中枢神经，主要职责包括：
 * 1. 初始化系统核心组件（如唤醒源配置）。
 * 2. 运行后台业务主循环 (Worker Loop)。
 * 3. 提供跨线程通信的静态接口 (sendToUI, sendToWorker)。
 * 4. 管理系统电源状态，实现自动休眠 (Light Sleep) 和唤醒逻辑。
 */
class SysController {
public:
    // ========================================================================
    // 生命周期管理
    // ========================================================================

    /**
     * @brief 初始化系统控制器
     * @details 
     * - 初始化活动计时器。
     * - 配置触摸屏中断引脚 (PIN_TOUCH_INT) 为唤醒源。
     * - 启用 GPIO 唤醒功能，允许从 Light Sleep 中被触摸唤醒。
     */
    static void setup();

    /**
     * @brief 系统主循环 (运行在 Worker 线程)
     * @details 
     * - 驱动 PageManager 的后台逻辑 (PageManager::loop)。
     * - 检查系统空闲时间，若超时 (如 30秒) 则自动进入 Light Sleep 模式。
     * - 负责在唤醒后恢复必要的系统状态。
     */
    static void loop();

    // ========================================================================
    // 事件处理
    // ========================================================================

    /**
     * @brief 处理来自 GUI 线程的指令
     * @param event 指向事件结构体的指针
     * @details 根据事件类型 (如 CMD_FETCH_WEATHER, CMD_SYSTEM_REBOOT) 执行相应的后台业务逻辑。
     */
    static void handleEvent(sys_event_t *event);

    // ========================================================================
    // 消息发送 (跨线程通信)
    // ========================================================================

    /**
     * @brief 发送指令给 Worker 线程 (通常由 GUI 线程调用)
     * @param type 事件类型 (SysEventType)
     * @param arg  整型参数 (可选，默认为 0)
     * @param payload 指针参数 (可选，默认为 NULL，用于传递复杂数据)
     */
    static void sendToWorker(SysEventType type, int32_t arg = 0, void* payload = NULL);

    /**
     * @brief 发送数据给 GUI 线程 (通常由 Worker 线程调用)
     * @param type 事件类型 (SysEventType)
     * @param arg  整型参数 (可选，默认为 0)
     * @param payload 指针参数 (可选，默认为 NULL)
     */
    static void sendToUI(SysEventType type, int32_t arg = 0, void* payload = NULL);

    // ========================================================================
    // 电源管理
    // ========================================================================

    /**
     * @brief 更新系统活动时间
     * @details 重置空闲计时器。应在检测到用户操作（触摸、按键）或关键后台任务活动时调用。
     */
    static void updateActivity();

    /**
     * @brief 检查是否应该进入休眠
     * @param timeout_ms 超时阈值 (毫秒)
     * @return true 如果当前空闲时间超过阈值
     * @return false 如果系统仍处于活跃状态
     */
    static bool shouldSleep(uint32_t timeout_ms);

private:
    /**
     * @brief 上次检测到系统活动的时间戳 (millis)
     */
    static uint32_t last_activity_time;
};

#endif
