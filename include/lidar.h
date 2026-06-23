#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//init UART1 (Default: D3/GPIO4 RX, D2/GPIO3 TX)
esp_err_t lidar_init(void);

//read distance in cm (returns 0 on error/low signal)
uint16_t lidar_read_distance(void);

#ifdef __cplusplus
}
#endif
