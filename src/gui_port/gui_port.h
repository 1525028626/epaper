#ifndef GUI_PORT_H
#define GUI_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

// 初始化 LVGL 框架及其与硬件的连接
void gui_port_init(void);

// 进入休眠前的 GUI 处理 (挂起触摸任务，关闭屏幕电源)
void gui_enter_sleep(void);

// 退出休眠后的 GUI 恢复 (恢复触摸任务，重置屏幕)
void gui_exit_sleep(void);



#ifdef __cplusplus
}
#endif

#endif // GUI_PORT_H