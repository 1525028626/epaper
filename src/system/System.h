#ifndef SYSTEM_H
#define SYSTEM_H

#include "SysEvent.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// 引用 main.cpp 中的全局队列
extern QueueHandle_t g_gui_queue;
extern QueueHandle_t g_worker_queue;

/**
 * @brief 系统级静态辅助类
 * 提供跨线程通信的便捷接口
 */
class System {
public:
    /**
     * @brief 发送指令给后台 Worker 线程 (App -> Worker)
     * @param type 事件类型 (SysEventType)
     * @param arg  整数参数 (可选)
     * @param payload 指针参数 (可选，需注意内存管理)
     */
    static void sendToWorker(SysEventType type, int32_t arg = 0, void* payload = NULL) {
        sys_event_t evt = {type, payload, arg};
        if(g_worker_queue) xQueueSend(g_worker_queue, &evt, 0);
    }

    /**
     * @brief 发送数据给 GUI 线程 (Worker -> App)
     * @param type 事件类型 (SysEventType)
     * @param arg  整数参数 (可选)
     * @param payload 指针参数 (可选)
     */
    static void sendToUI(SysEventType type, int32_t arg = 0, void* payload = NULL) {
        sys_event_t evt = {type, payload, arg};
        if(g_gui_queue) xQueueSend(g_gui_queue, &evt, 0);
    }
};

#endif