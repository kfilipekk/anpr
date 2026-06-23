#pragma once
#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    bool plate_found;
    char plate_text[16];
    float confidence;
    char region[16];
} plate_result_t;

esp_err_t plate_uploader_send(uint8_t *buf, size_t len, plate_result_t *result);
esp_err_t plate_uploader_send_from_sd(const char *path, plate_result_t *result);
