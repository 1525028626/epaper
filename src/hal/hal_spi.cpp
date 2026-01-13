#include "hal/hal_spi.h"
#include "common/board_pins.h"
#include <Arduino.h>

/* ==================================================================
 * 软件模拟 SPI (Software Bit-Banging)
 * 完全复刻 Display_EPD_W21_spi.cpp 的时序逻辑
 * ================================================================== */

static bool g_spi_inited = false;

sys_status_t hal_spi_init(void) {
    if (g_spi_inited) return SYS_OK;

    // 配置引脚为推挽输出
    pinMode(PIN_SPI_SCK, OUTPUT);
    pinMode(PIN_SPI_MOSI, OUTPUT);
    pinMode(PIN_SPI_CS, OUTPUT);

    // 初始状态：CS拉高(不选中)，CLK拉低(Mode0 空闲电平)
    digitalWrite(PIN_SPI_CS, HIGH);
    digitalWrite(PIN_SPI_SCK, LOW);

    g_spi_inited = true;
    return SYS_OK;
}

// 核心：完全照搬原厂 SPI_Write 逻辑
sys_status_t hal_spi_write_byte(uint8_t data) {
    if (!g_spi_inited) return SYS_ERR_INVALID_ARG;

    // 循环发送 8 位 (MSB First)
    // 1. CLK 拉低 (准备数据)
    digitalWrite(PIN_SPI_SCK, LOW);
    for (uint8_t i = 0; i < 8; i++) {
 
        // 2. 设置 MOSI 电平
        if (data & 0x80) {
            digitalWrite(PIN_SPI_MOSI, HIGH);
        } else {
            digitalWrite(PIN_SPI_MOSI, LOW);
        }
        digitalWrite(PIN_SPI_SCK, HIGH);
        digitalWrite(PIN_SPI_SCK, LOW);
        // 3. 移位
        data = (data << 1);
        

    }


    return SYS_OK;
}

// 缓冲区发送：简单的循环调用
sys_status_t hal_spi_write_buffer(const uint8_t *data, uint32_t len) {
    if (!g_spi_inited || data == NULL) return SYS_ERR_INVALID_ARG;

    for (uint32_t i = 0; i < len; i++) {
        hal_spi_write_byte(data[i]);
    }
    return SYS_OK;
}