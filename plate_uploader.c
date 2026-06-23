#include "plate_uploader.h"
#include "esp_log.h"

static const char *TAG = "UPLOADER";

esp_err_t plate_uploader_send(uint8_t *buf, size_t len, plate_result_t *res) {
    ESP_LOGI(TAG, "Uploading %zu bytes", len);
    res->plate_found = false;
    return ESP_OK;
}

esp_err_t plate_uploader_send_from_sd(const char *path, plate_result_t *res) {
    res->plate_found = false;
    return ESP_OK;
}
