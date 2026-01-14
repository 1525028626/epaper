#ifndef SYS_EVENT_H
#define SYS_EVENT_H

#include <Arduino.h>

/**
 * @file SysEvent.h
 * @brief 系统事件定义文件
 * @details 定义了跨线程通信的消息格式和事件类型枚举。
 */

/**
 * @enum SysEventType
 * @brief 系统事件类型枚举 (全局唯一 ID)
 * @details 定义了 GUI 线程和 Worker 线程之间通信的所有消息类型。
 */
enum SysEventType {
    EVT_NONE = 0, ///< 空事件

    // ==========================================
    // Worker -> GUI (系统通知/数据推送)
    // ==========================================
    EVT_WIFI_CONNECTED,      ///< WiFi 已连接
    EVT_WIFI_DISCONNECTED,   ///< WiFi 已断开
    EVT_TIME_UPDATED,        ///< 系统时间已更新 (通常每分钟一次)
    EVT_DATA_WEATHER,        ///< 天气数据到达 (payload: 通常是 int 温度值)

    // ==========================================
    // GUI -> Worker (控制指令)
    // ==========================================
    CMD_WIFI_CONNECT,      ///< 请求连接 WiFi
    CMD_FETCH_WEATHER,     ///< 请求获取最新天气数据
    CMD_SYSTEM_REBOOT      ///< 请求系统重启
};

/**
 * @struct sys_event_t
 * @brief 通用系统事件结构体
 * @details 用于 FreeRTOS 队列传输的消息载体。
 */
typedef struct {
    SysEventType type;     ///< 事件类型
    void* payload;         ///< 复杂数据指针 (可选)。注意：如果是动态分配的内存，接收方负责 delete。
    int32_t arg;           ///< 简单整数参数 (可选)。用于传递状态码、数值等简单数据。
} sys_event_t;

#endif
