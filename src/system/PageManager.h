#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include "AppBase.h"

/**
 * @file PageManager.h
 * @brief 页面管理器头文件
 * @details 定义了 PageManager 类，负责管理 App 的生命周期（加载、运行、销毁）。
 */

/**
 * @class PageManager
 * @brief 页面管理器 (静态工具类)
 * 
 * @details
 * 类似于 Android 的 ActivityManager 或 iOS 的 UINavigationController。
 * 它维护一个当前正在运行的 App 实例指针 (`currentApp`)。
 * 
 * 主要功能：
 * 1. 切换 App (`loadApp`)：自动处理旧 App 的 `onStop` 和析构，新 App 的 `onStart`。
 * 2. 事件分发 (`handleEvent`)：将系统事件路由给当前 App。
 * 3. 后台循环 (`loop`)：维持当前 App 的后台任务。
 */
class PageManager {
public:
    /**
     * @brief 加载并切换到新的 App
     * @param newApp 新 App 的实例指针 (必须是 new 分配的堆内存)
     * @note 
     * - 此函数会自动 `delete` 当前正在运行的 App 实例，防止内存泄漏。
     * - 调用示例: `PageManager::loadApp(new App_Home());`
     */
    static void loadApp(AppBase* newApp);
    
    /**
     * @brief 将系统事件分发给当前 App
     * @param event 指向事件结构体的指针
     * @details 如果当前有 App 在运行，调用其 `onEvent` 方法。
     */
    static void handleEvent(sys_event_t* event);
    
    /**
     * @brief 执行当前 App 的后台主循环
     * @details 应该在 Worker 线程的循环中被周期性调用。
     */
    static void loop();
    
    /**
     * @brief 获取当前正在运行的 App 实例
     * @return AppBase* 指针，如果没有 App 运行则返回 nullptr
     */
    static AppBase* getCurrentApp() { return currentApp; }

private:
    /**
     * @brief 当前运行的 App 实例指针
     */
    static AppBase* currentApp;
};

#endif
