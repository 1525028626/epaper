#ifndef APP_HOME_H
#define APP_HOME_H

#include "../system/AppBase.h"

/**
 * @brief 主页应用
 * 显示天气、时间等信息
 */
class App_Home : public AppBase {
public:
    void onStart() override;
    void onStop() override;
    void onEvent(sys_event_t* event) override;
};

#endif