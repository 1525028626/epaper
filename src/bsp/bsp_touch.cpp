#include "bsp/bsp_touch.h"
#include "hal/hal_i2c.h"
#include "hal/hal_gpio.h"
#include "common/board_pins.h"
#include "bsp/bsp_epd.h" 
#include <Arduino.h>
#include <Wire.h> // 引入 Wire 以便直接配置灵敏度


/**
 * @brief 初始化触摸屏 (I2C)
 */
sys_status_t bsp_touch_init(void) {
    // 复位触摸芯片
    pinMode(PIN_TOUCH_RST, OUTPUT);
    digitalWrite(PIN_TOUCH_RST, LOW);
    delay(20);
    digitalWrite(PIN_TOUCH_RST, HIGH);
    delay(200);

    // 初始化 I2C
    // 降低速度到 100kHz 确保稳定
    Wire.begin(PIN_TOUCH_SDA, PIN_TOUCH_SCL, 100000); 

    return SYS_OK;
}

/**
 * @brief 读取触摸点坐标
 * @param point 输出参数，存储坐标和状态
 * @return true 读取成功且有触摸, false 失败或无触摸
 */
bool bsp_touch_read(touch_point_t *point) {
    if (point == NULL) return false;

    Wire.beginTransmission(FT_TOUCH_I2C_ADDR);
    Wire.write(FT_REG_TD_STATUS);
    if (Wire.endTransmission() != 0) {
        // I2C 通信失败 (设备没接好?)
        return false;
    }

    uint8_t len = Wire.requestFrom(FT_TOUCH_I2C_ADDR, 5);
    if (len != 5) {
        return false;
    }

    uint8_t buf[5];
    for (int i = 0; i < 5; i++) buf[i] = Wire.read();

    // 判断触摸点数量 (低 4 位)
    uint8_t touches = buf[0] & 0x0F;
    if (touches == 0 || touches > 2) {
        point->pressed = false;
        return false;
    }

    // 读取坐标 (FT6336 标准协议)
    // buf[1] & 0x0F 是 X 的高 4 位
    uint16_t raw_x = ((buf[1] & 0x0F) << 8) | buf[2];
    uint16_t raw_y = ((buf[3] & 0x0F) << 8) | buf[4];

    // 赋值
    point->x = raw_x;
    point->y = raw_y;
    point->pressed = true;

    // 【调试信息】
    // 打开串口监视器，如果你看到这里有输出，说明硬件是好的，问题在 LVGL 映射
    // Serial.printf("Touch Raw: x=%d, y=%d\n", raw_x, raw_y); // 注释掉，避免阻塞

    return true;
}