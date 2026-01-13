#ifndef APP_BASE_H
#define APP_BASE_H

#include "SysEvent.h"
#include "System.h"
#include <lvgl.h>

/**
 * @brief App 基类 (接口)
 * 所有具体的页面/应用都应继承此类，并实现生命周期方法
 */
class AppBase {
public:
    virtual ~AppBase() {}

    // [生命周期] App 启动: 初始化 UI, 绑定事件
    virtual void onStart() = 0;

    // [生命周期] App 退出: 销毁 UI, 释放内存
    virtual void onStop() = 0;

    // [事件] 处理来自后台的消息
    virtual void onEvent(sys_event_t* event) {}

    // [后台] 轮询逻辑 (可选)
    virtual void onRunningLoop() {} 
};

#endif