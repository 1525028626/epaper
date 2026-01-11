/* 文件路径: src/hal/hal_gpio.cpp */
#include "hal_gpio.h"
#include <Arduino.h>

void hal_gpio_init(uint32_t pin, hal_gpio_mode_t mode) {
    uint8_t arduino_mode;
    switch (mode) {
        case HAL_GPIO_MODE_OUTPUT: 
            arduino_mode = OUTPUT; 
            break;
        case HAL_GPIO_MODE_INPUT_PULLUP: 
            arduino_mode = INPUT_PULLUP; 
            break;
        case HAL_GPIO_MODE_INPUT_PULLDOWN: 
            arduino_mode = INPUT_PULLDOWN; 
            break;
        default: 
            arduino_mode = INPUT; 
            break;
    }
    pinMode(pin, arduino_mode);
}

void hal_gpio_write(uint32_t pin, hal_gpio_state_t state) {
    digitalWrite(pin, (state == HAL_GPIO_HIGH) ? HIGH : LOW);
}

void hal_gpio_toggle(uint32_t pin) {
    digitalWrite(pin, !digitalRead(pin));
}

hal_gpio_state_t hal_gpio_read(uint32_t pin) {
    return (digitalRead(pin) == HIGH) ? HAL_GPIO_HIGH : HAL_GPIO_LOW;
}