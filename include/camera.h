#pragma once
#include "esp_err.h"

esp_err_t camera_init(void);
void camera_set_motion_res(void);
void camera_set_capture_res(void);
