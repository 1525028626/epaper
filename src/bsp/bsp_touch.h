#ifndef BSP_TOUCH_H
#define BSP_TOUCH_H

#include "common/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t x;
    uint16_t y;
    bool pressed;
} touch_point_t;

// 初始化触摸屏
sys_status_t bsp_touch_init(void);

// 读取坐标
bool bsp_touch_read(touch_point_t *point);

#ifdef __cplusplus
}
#endif

#endif // BSP_TOUCH_H