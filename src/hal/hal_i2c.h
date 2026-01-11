#ifndef HAL_I2C_H
#define HAL_I2C_H

#include "common/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// 初始化 I2C 总线
sys_status_t hal_i2c_init(void);

// 读取寄存器 (写寄存器地址 -> 重启总线 -> 读取数据)
sys_status_t hal_i2c_read_mem(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint32_t len);

// 写寄存器 (写寄存器地址 -> 写数据)
sys_status_t hal_i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // HAL_I2C_H