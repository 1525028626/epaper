#ifndef BSP_EPD_H
#define BSP_EPD_H

#include "common/types.h"

// 依据 Display_EPD_W21.h
#define EPD_WIDTH   176
#define EPD_HEIGHT  264

#ifdef __cplusplus
extern "C" {
#endif

sys_status_t bsp_epd_init(void);
void bsp_epd_display_full(const uint8_t *image_buffer);
void bsp_epd_clear(uint8_t color);
void bsp_epd_sleep(void);

#ifdef __cplusplus
}
#endif

#endif