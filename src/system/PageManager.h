#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include "AppBase.h"

/**
 * @brief 页面管理器 (静态类)
 * 负责管理当前运行的 App 生命周期 (启动、停止、事件分发)
 */
class PageManager {
public:
    // 切换到新 App (自动销毁旧 App 并释放内存)
    static void loadApp(AppBase* newApp);
    
    // 分发事件给当前 App
    static void handleEvent(sys_event_t* event);
    
    // 执行当前 App 的后台循环逻辑
    static void loop();
    
    // 获取当前 App 指针
    static AppBase* getCurrentApp() { return currentApp; }

private:
    static AppBase* currentApp;
};

#endif