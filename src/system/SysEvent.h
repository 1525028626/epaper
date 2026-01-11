#ifndef SYS_EVENT_H
#define SYS_EVENT_H

#include <Arduino.h>

// 事件类型
enum EventType {
    // 系统级事件
    EVT_NONE,
    EVT_WIFI_CONNECTED,
    EVT_WIFI_DISCONNECTED,
    
    // UI 指令 (Worker -> GUI)
    EVT_UPDATE_WEATHER_DATA,  // 后台下发天气数据
    EVT_UPDATE_WORD_DATA,     // 后台下发单词数据
    
    // 业务指令 (GUI -> Worker)
    CMD_FETCH_WEATHER,        // UI 请求刷新天气
    CMD_LOAD_NEXT_WORD,       // UI 请求下一个单词
};

// 通用事件结构体
typedef struct {
    EventType type;
    void* payload;       // 指向数据的指针 (需注意内存释放)
    size_t len;          // 数据长度/附加参数
} sys_event_t;

#endif