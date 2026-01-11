#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include "system/AppBase.h"
#include "system/SysEvent.h"

class PageManager {
public:
    /**
     * @brief 切换到新的 App
     * * @note  这个函数通常在 GUI 线程中调用（例如点击桌面图标时）。
     * 它会先销毁(delete) 当前运行的 App，然后启动新的 App。
     * * @param newApp 指向新 App 实例的指针 (e.g. new App_Weather())
     */
    static void loadApp(AppBase* newApp);

    /**
     * @brief 将系统事件分发给当前 App
     * * @note  必须在 GUI 线程 (Core 1) 中调用。
     * 它会调用 currentApp->onEvent()，允许 App 更新 UI。
     * * @param event 事件指针
     */
    static void handleEvent(sys_event_t* event);

    /**
     * @brief 执行当前 App 的后台逻辑
     * * @note  必须在 Worker 线程 (Core 0) 中循环调用。
     * 它会调用 currentApp->onRunningLoop()。
     */
    static void loop();

    /**
     * @brief 获取当前正在运行的 App 指针 (仅限高级用法)
     */
    static AppBase* getCurrentApp();

private:
    // 持有当前正在运行的 App 实例
    static AppBase* currentApp;
};

#endif // PAGE_MANAGER_H