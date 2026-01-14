#ifndef APP_BASE_H
#define APP_BASE_H

#include "SysEvent.h"
#include "SysController.h"
#include <lvgl.h>

/**
 * @file AppBase.h
 * @brief App 基类定义文件
 * @details 定义了所有 App 必须遵循的接口规范。
 */

/**
 * @class AppBase
 * @brief 应用基类 (抽象类)
 * 
 * @details
 * 所有的页面应用 (如 App_Home, App_Setting) 都必须继承此类。
 * 它定义了 App 的标准生命周期：
 * - `onStart()`: 启动时调用 (创建 UI，申请资源)
 * - `onStop()`: 停止时调用 (销毁 UI，释放资源)
 * - `onEvent()`: 处理系统事件
 * - `onRunningLoop()`: 后台轮询
 */
class AppBase {
public:
    /**
     * @brief 虚析构函数
     * @details 确保通过基类指针删除派生类对象时，派生类的析构函数能被正确调用。
     */
    virtual ~AppBase() {}

    /**
     * @brief [生命周期] App 启动
     * @details 
     * 在 App 被 PageManager 加载时调用。
     * 通常在此处进行：
     * 1. 调用 LVGL 的 screen_init 函数初始化 UI。
     * 2. 加载初始数据。
     * 3. 发送初始网络请求。
     */
    virtual void onStart() = 0;

    /**
     * @brief [生命周期] App 停止
     * @details 
     * 在 App 被切换（销毁）前调用。
     * 必须在此处进行：
     * 1. 销毁 LVGL 对象 (`lv_obj_del`)，释放 RAM。
     * 2. 停止未完成的定时器或任务。
     */
    virtual void onStop() = 0;

    /**
     * @brief [事件] 处理系统事件
     * @param event 接收到的事件
     * @details 
     * 处理从 Worker 线程发来的数据 (如天气更新) 或其他系统通知。
     * 默认实现为空。
     */
    virtual void onEvent(sys_event_t* event) {}

    /**
     * @brief [后台] 后台轮询逻辑
     * @details 
     * 会在 Worker 线程的循环中被高频调用。
     * 可用于处理非阻塞的后台任务。
     * 默认实现为空。
     */
    virtual void onRunningLoop() {} 
};

#endif
