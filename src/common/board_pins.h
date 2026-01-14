/**
 * @file board_pins.h
 * @brief 硬件引脚定义文件
 * @details 定义了 ESP32-S3 与外设 (EPD, Touch, SPI, I2C) 的连接引脚。
 *          适用于 ESP32-S3 开发板。
 */
#ifndef BOARD_PINS_H
#define BOARD_PINS_H

#include <Arduino.h>

/* ==================================================================
 * SPI 总线 / EPD 屏幕接口
 * 使用 VSPI (FSPI) 或 HSPI
 * ================================================================== */
#define PIN_SPI_SCK     10  ///< SPI 时钟
#define PIN_SPI_MOSI    9   ///< SPI 主出从入
#define PIN_SPI_MISO    -1  ///< SPI 主入从出 (电子纸通常不需要读，故悬空)
#define PIN_SPI_CS      11  ///< SPI 片选

#define PIN_EPD_DC      4   ///< 数据/命令选择 (High=Data, Low=Cmd)
#define PIN_EPD_RST     5   ///< 外部复位 (低电平有效)
#define PIN_EPD_BUSY    6   ///< 忙状态指示 (High=Busy)

/* ==================================================================
 * I2C 总线 / Touch 触摸屏 (FT6336)
 * ================================================================== */
#define PIN_TOUCH_SCL   21  ///< I2C 时钟
#define PIN_TOUCH_SDA   47  ///< I2C 数据
#define PIN_TOUCH_RST   14  ///< 触摸复位
#define PIN_TOUCH_INT   13  ///< 触摸中断 (低电平触发)

// FT6336 默认 I2C 地址通常是 0x38
#define FT_TOUCH_I2C_ADDR 0x38
#define FT_REG_TD_STATUS  0x02

#endif // BOARD_PINS_H