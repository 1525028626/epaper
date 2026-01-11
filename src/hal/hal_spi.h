#ifndef HAL_SPI_H
#define HAL_SPI_H

#include "common/types.h"

#ifdef __cplusplus
extern "C" {
#endif

sys_status_t hal_spi_init(void);
sys_status_t hal_spi_write_byte(uint8_t data);
sys_status_t hal_spi_write_buffer(const uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // HAL_SPI_H