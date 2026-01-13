#include "PageManager.h"

AppBase* PageManager::currentApp = nullptr;

/**
 * @brief 加载新应用
 * @param newApp 新应用的实例指针 (需由调用者 new 出来)
 */
void PageManager::loadApp(AppBase* newApp) {
    if (newApp == nullptr) return;
    if (currentApp == newApp) return;

    // 1. 销毁旧 App (释放内存!)
    if (currentApp != nullptr) {
        currentApp->onStop();
        delete currentApp; 
        currentApp = nullptr;
    }

    // 2. 启动新 App
    currentApp = newApp;
    currentApp->onStart();
}

/**
 * @brief 将系统事件分发给当前应用
 */
void PageManager::handleEvent(sys_event_t* event) {
    if (currentApp) currentApp->onEvent(event);
}

/**
 * @brief 周期性调用当前应用的后台逻辑
 */
void PageManager::loop() {
    if (currentApp) currentApp->onRunningLoop();
}