#ifndef APP_BASE_H
#define APP_BASE_H

#include <lvgl.h>
#include "system/SysEvent.h"

class AppBase {
public:
    virtual ~AppBase() {}

    // [生命周期] App 被打开时调用 (初始化 UI，注册事件)
    virtual void onStart() = 0;

    // [生命周期] App 被关闭时调用 (销毁 UI，释放内存)
    virtual void onStop() = 0;

    // [事件] 当收到事件队列消息时调用
    virtual void onEvent(sys_event_t* event) {}

    // [后台] 如果 App 需要在 Worker 线程跑逻辑 (可选)
    virtual void onRunningLoop() {} 
};

#endif