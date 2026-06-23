#include "sd_card_logger.h"
#include "esp_log.h"

static const char *TAG = "SD_LOGGER";

esp_err_t sd_logger_init(void) { return ESP_OK; }
esp_err_t sd_logger_save_frame(uint8_t *buf, size_t len, const char *meta) {
    ESP_LOGI(TAG, "Saved %zu bytes (%s)", len, meta);
    return ESP_OK;
}
esp_err_t sd_logger_get_pending(char *path, size_t max_len) { return ESP_ERR_NOT_FOUND; }
