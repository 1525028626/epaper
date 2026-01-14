#include "PageManager.h"

/**
 * @file PageManager.cpp
 * @brief 页面管理器实现文件
 */

// 初始化静态成员变量
AppBase* PageManager::currentApp = nullptr;

/**
 * @brief 加载新应用
 * @param newApp 新应用的实例指针
 */
void PageManager::loadApp(AppBase* newApp) {
    // 安全检查
    if (newApp == nullptr) return;
    
    // 防止重复加载同一个实例
    if (currentApp == newApp) return;

    // 1. 销毁旧 App (如果存在)
    if (currentApp != nullptr) {
        // 调用生命周期方法：停止
        currentApp->onStop();
        
        // 释放内存
        delete currentApp; 
        currentApp = nullptr;
    }

    // 2. 启动新 App
    currentApp = newApp;
    
    // 调用生命周期方法：启动
    currentApp->onStart();
}

/**
 * @brief 将系统事件分发给当前应用
 * @param event 系统事件
 */
void PageManager::handleEvent(sys_event_t* event) {
    if (currentApp) {
        currentApp->onEvent(event);
    }
}

/**
 * @brief 周期性调用当前应用的后台逻辑
 */
void PageManager::loop() {
    if (currentApp) {
        currentApp->onRunningLoop();
    }
}
