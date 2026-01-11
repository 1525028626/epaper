#include "system/PageManager.h"

AppBase* PageManager::currentApp = nullptr;

void PageManager::loadApp(AppBase* newApp) {
    if (currentApp == newApp) return;

    // 1. 停止当前 App
    if (currentApp != nullptr) {
        currentApp->onStop();
        delete currentApp; // 假设 App 对象是动态 new 出来的
    }

    // 2. 启动新 App
    currentApp = newApp;
    if (currentApp != nullptr) {
        currentApp->onStart();
    }
}

void PageManager::handleEvent(sys_event_t* event) {
    if (currentApp != nullptr) {
        currentApp->onEvent(event);
    }
}

// 在 Worker 任务中调用
void PageManager::loop() {
    if (currentApp != nullptr) {
        currentApp->onRunningLoop();
    }
}