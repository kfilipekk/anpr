#pragma once
#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>

esp_err_t sd_logger_init(void);
esp_err_t sd_logger_save_frame(uint8_t *buf, size_t len, const char *metadata);
esp_err_t sd_logger_get_pending(char *path, size_t max_len);
