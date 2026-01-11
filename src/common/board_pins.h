#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include <Arduino.h>

/* --- SPI / EPD 部分 (保持不变) --- */
#define PIN_SPI_SCK     10
#define PIN_SPI_MOSI    9
#define PIN_SPI_MISO    -1
#define PIN_SPI_CS      11

#define PIN_EPD_DC      4
#define PIN_EPD_RST     5
#define PIN_EPD_BUSY    6

/* --- Touch (FT6336) 部分 --- */
#define PIN_TOUCH_SCL   21
#define PIN_TOUCH_SDA   47
#define PIN_TOUCH_RST   14
#define PIN_TOUCH_INT   13

// FT6336 默认 I2C 地址通常是 0x38
#define FT_TOUCH_I2C_ADDR 0x38
#define FT_REG_TD_STATUS  0x02

#endif // BOARD_PINS_H