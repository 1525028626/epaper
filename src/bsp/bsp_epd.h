/**
 * @file bsp_epd.h
 * @brief 电子纸显示屏 (EPD) 板级支持包头文件
 * @details 定义了电子纸的初始化、全屏刷新、清屏和睡眠等接口。
 *          适配型号：2.7寸 EPD (W21 驱动)
 */
#ifndef BSP_EPD_H
#define BSP_EPD_H

#include "common/types.h"

// 依据 Display_EPD_W21.h
// 定义屏幕分辨率
#define EPD_WIDTH   176  ///< 屏幕宽度 (像素)
#define EPD_HEIGHT  264  ///< 屏幕高度 (像素)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化电子纸
 * @return sys_status_t 初始化状态
 */
sys_status_t bsp_epd_init(void);

/**
 * @brief 全屏显示图像
 * @param image_buffer 图像数据缓冲区指针 (1 bit/pixel)
 */
void bsp_epd_display_full(const uint8_t *image_buffer);

/**
 * @brief 清空屏幕
 * @param color 填充颜色 (0: 黑色, 1: 白色)
 */
void bsp_epd_clear(uint8_t color);

/**
 * @brief 让电子纸进入深度睡眠模式
 * @details 降低功耗，唤醒通常需要重新初始化
 */
void bsp_epd_sleep(void);

#ifdef __cplusplus
}
#endif

#endif // BSP_EPD_H