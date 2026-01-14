#ifndef APP_HOME_H
#define APP_HOME_H

#include "../system/AppBase.h"

/**
 * @file app_home.h
 * @brief 主页应用头文件
 * @details 定义了系统启动后的默认首页应用。
 */

/**
 * @class App_Home
 * @brief 主页应用类
 * 
 * @details 
 * 继承自 AppBase，实现了首页的业务逻辑。
 * 主要功能：
 * 1. 显示主界面 (ui_HomePage)。
 * 2. 启动时自动请求天气数据。
 * 3. 接收并展示天气、时间更新。
 */
class App_Home : public AppBase {
public:
    /**
     * @brief [生命周期] 启动
     * @details 初始化主页 UI，请求天气数据。
     */
    void onStart() override;

    /**
     * @brief [生命周期] 停止
     * @details 销毁主页 UI 资源。
     */
    void onStop() override;

    /**
     * @brief [事件] 处理系统事件
     * @param event 系统事件 (如天气更新)
     */
    void onEvent(sys_event_t* event) override;
};

#endif
