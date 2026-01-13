#ifndef SYS_EVENT_H
#define SYS_EVENT_H

#include <Arduino.h>

/**
 * @brief 系统事件类型枚举 (全局唯一 ID)
 */
enum SysEventType {
    EVT_NONE = 0,

    // --- [系统通知] (Worker -> GUI) ---
    EVT_WIFI_CONNECTED,
    EVT_WIFI_DISCONNECTED,
    EVT_TIME_UPDATED,      // 每一分钟更新一次时间
    EVT_DATA_WEATHER,      // 天气数据到达 (payload: int 温度)

    // --- [控制指令] (GUI -> Worker) ---
    CMD_WIFI_CONNECT,      // 请求连接 WiFi
    CMD_FETCH_WEATHER,     // 请求获取天气
    CMD_SYSTEM_REBOOT      // 请求重启
};

/**
 * @brief 通用事件结构体，用于队列传输
 */
typedef struct {
    SysEventType type;
    void* payload;         // 复杂数据指针 (如果是 new 出来的，接收方记得 delete)
    int32_t arg;           // 简单参数 (整数/状态码)
} sys_event_t;

#endif