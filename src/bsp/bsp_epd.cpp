#include "bsp_epd.h"
#include "hal/hal_spi.h"
#include "hal/hal_gpio.h"
#include "common/board_pins.h"
#include <Arduino.h> 

#define EPD_BUSY_TIMEOUT_MS  5000

/* --- 内部辅助函数 --- */

/**
 * @brief 等待电子纸忙闲状态 (BUSY 引脚)
 */
static void _epd_wait_busy(void) {
    uint32_t start_time = millis();
    // 原厂逻辑：HIGH = BUSY
    while (hal_gpio_read(PIN_EPD_BUSY) == HAL_GPIO_HIGH) {
        delay(1); 
        if ((millis() - start_time) > EPD_BUSY_TIMEOUT_MS) break; 
    }
}

/**
 * @brief 硬件复位电子纸
 */
static void _epd_reset(void) {
    // 复刻 EPD_HW_Init 的复位时序
    hal_gpio_write(PIN_EPD_RST, HAL_GPIO_LOW);
    delay(20); // 原厂是 delay_xms(10) * 2 roughly
    hal_gpio_write(PIN_EPD_RST, HAL_GPIO_HIGH);
    delay(20);
}

/**
 * @brief 发送命令 (DC = LOW)
 */
static void _epd_cmd(uint8_t cmd) {
    hal_gpio_write(PIN_SPI_CS, HAL_GPIO_LOW);
    hal_gpio_write(PIN_EPD_DC, HAL_GPIO_LOW); // Command
    hal_spi_write_byte(cmd); // 使用软件模拟SPI
    hal_gpio_write(PIN_SPI_CS, HAL_GPIO_HIGH);
}

/**
 * @brief 发送数据 (DC = HIGH)
 */
static void _epd_data(uint8_t data) {
    hal_gpio_write(PIN_SPI_CS, HAL_GPIO_LOW);
    hal_gpio_write(PIN_EPD_DC, HAL_GPIO_HIGH); // Data
    hal_spi_write_byte(data); // 使用软件模拟SPI
    hal_gpio_write(PIN_SPI_CS, HAL_GPIO_HIGH);
}

/* --- 公开接口 --- */

/**
 * @brief 初始化电子纸硬件 (GPIO, SPI, 复位)
 */
sys_status_t bsp_epd_init(void) {
    hal_gpio_init(PIN_EPD_RST, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_init(PIN_EPD_DC,  HAL_GPIO_MODE_OUTPUT);
    hal_gpio_init(PIN_EPD_BUSY, HAL_GPIO_MODE_INPUT); 
    
    // 必须确保 SPI 引脚已配置 (软件模拟模式)
    hal_spi_init();

    _epd_reset();
    _epd_wait_busy();

    _epd_cmd(0x12); // SWRESET
    _epd_wait_busy();

    // 原厂这里没有任何其他指令，依靠默认值工作
    
    return SYS_OK;
}

/**
 * @brief 全屏刷新显示
 * @param image_buffer 图像数据指针 (1bit per pixel)
 */
void bsp_epd_display_full(const uint8_t *image_buffer) {
    if (image_buffer == NULL) return;

    // 1. 写 RAM 命令
    _epd_cmd(0x24); 
    
    // 2. 写入数据
    // 176 * 264 / 8 = 5808 字节
    uint32_t total_bytes = (EPD_WIDTH / 8) * EPD_HEIGHT;

    for (uint32_t i = 0; i < total_bytes; i++) {
        // 原厂 Epaper_Write_Data 内部包含了 CS 翻转
        // 所以我们逐字节调用 _epd_data
        _epd_data(image_buffer[i]);
    }

    // 3. 刷新序列
    _epd_cmd(0x22); 
    _epd_data(0xF7); 
    _epd_cmd(0x20); 
    _epd_wait_busy();
}

/**
 * @brief 清屏 (填充指定颜色)
 */
void bsp_epd_clear(uint8_t color) {
    _epd_cmd(0x24);

    
    for (uint32_t i = 0; i <5808 ; i++) {
        _epd_data(color);
    }

    _epd_cmd(0x22); 
    _epd_data(0xF7); 
    _epd_cmd(0x20); 
    _epd_wait_busy();
}

/**
 * @brief 进入深度睡眠模式
 */
void bsp_epd_sleep(void) {
    _epd_cmd(0x10); 
    _epd_data(0x01);
}