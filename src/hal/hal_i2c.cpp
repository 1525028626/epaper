#include "hal/hal_i2c.h"
#include "common/board_pins.h"
#include <Arduino.h>
#include <Wire.h>

#define I2C_FREQ 100000 // 100KHz

static bool g_i2c_inited = false;

sys_status_t hal_i2c_init(void) {
    if (g_i2c_inited) return SYS_OK;

    // 针对 ESP32-S3，显式指定 SDA/SCL
    if (!Wire.begin(PIN_TOUCH_SDA, PIN_TOUCH_SCL, I2C_FREQ)) {
        return SYS_FAIL;
    }
    
    g_i2c_inited = true;
    return SYS_OK;
}

sys_status_t hal_i2c_read_mem(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint32_t len) {
    if (!g_i2c_inited) return SYS_ERR_INVALID_ARG;

    // 1. 发送寄存器地址
    Wire.beginTransmission(dev_addr);
    Wire.write(reg_addr);
    if (Wire.endTransmission(false) != 0) { // false = 发送 Restart 信号，不释放总线
        return SYS_FAIL;
    }

    // 2. 请求数据
    if (Wire.requestFrom(dev_addr, (size_t)len) != len) {
        return SYS_ERR_TIMEOUT;
    }

    // 3. 读取数据
    for (size_t i = 0; i < len; i++) {
        data[i] = Wire.read();
    }

    return SYS_OK;
}

sys_status_t hal_i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
    if (!g_i2c_inited) return SYS_ERR_INVALID_ARG;

    Wire.beginTransmission(dev_addr);
    Wire.write(reg_addr);
    Wire.write(data);
    if (Wire.endTransmission() != 0) {
        return SYS_FAIL;
    }
    return SYS_OK;
}