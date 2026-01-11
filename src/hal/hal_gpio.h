/* 文件路径: include/hal/hal_gpio.h */
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "common/types.h" // 确保引用了之前定义的通用类型

#ifdef __cplusplus
extern "C" {
#endif

// 定义 GPIO 模式
typedef enum {
    HAL_GPIO_MODE_INPUT = 0,
    HAL_GPIO_MODE_OUTPUT,
    HAL_GPIO_MODE_INPUT_PULLUP,
    HAL_GPIO_MODE_INPUT_PULLDOWN
} hal_gpio_mode_t;

// 定义 GPIO 电平
typedef enum {
    HAL_GPIO_LOW = 0,
    HAL_GPIO_HIGH = 1
} hal_gpio_state_t;

/**
 * @brief 初始化 GPIO 引脚
 */
void hal_gpio_init(uint32_t pin, hal_gpio_mode_t mode);

/**
 * @brief 设置 GPIO 电平
 */
void hal_gpio_write(uint32_t pin, hal_gpio_state_t state);

/**
 * @brief 翻转 GPIO 电平
 */
void hal_gpio_toggle(uint32_t pin);

/**
 * @brief 读取 GPIO 电平
 */
hal_gpio_state_t hal_gpio_read(uint32_t pin);

#ifdef __cplusplus
}
#endif

#endif // HAL_GPIO_H