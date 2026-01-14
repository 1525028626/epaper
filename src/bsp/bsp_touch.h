/**
 * @file bsp_touch.h
 * @brief 触摸屏 (Touch Screen) 板级支持包头文件
 * @details 定义了触摸屏的初始化和坐标读取接口。
 *          适配型号：FT6336 或类似 I2C 接口电容触摸芯片。
 */
#ifndef BSP_TOUCH_H
#define BSP_TOUCH_H

#include "common/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 触摸点结构体
 */
typedef struct {
    uint16_t x;     ///< X 坐标
    uint16_t y;     ///< Y 坐标
    bool pressed;   ///< 是否按下
} touch_point_t;

/**
 * @brief 初始化触摸屏
 * @details 复位芯片并配置 I2C 接口。
 * @return sys_status_t 初始化状态
 */
sys_status_t bsp_touch_init(void);

/**
 * @brief 读取触摸点坐标
 * @param point 输出参数，存储坐标和状态
 * @return true 读取成功且有触摸, false 失败或无触摸
 */
bool bsp_touch_read(touch_point_t *point);

#ifdef __cplusplus
}
#endif

#endif // BSP_TOUCH_H