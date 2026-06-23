/*
 * motion_task (Core 0): Captures continuous QVGA and checks pixel differences, grabs an SVGA frame on trigger and queues it
 *
 * upload_task (Core 1): dequeues frames and POSTs them to Plate Recogniser, saves frames to SD card if Wi-Fi fails.
 *
 * retry_task (Core 1): drains pending SD files asynchronously when Wi-Fi restores.
*/

#include "anpr_config.h"
#include "camera.h"
#include "lidar.h"
#include "plate_uploader.h"
#include "sd_card_logger.h"

#include "esp_log.h"
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <string.h>
#include <stdio.h>

#include "src/application/SMARTConnect_main.h"
#include "src/protocol/mqtt/SMARTConnect_mqtt.h"

static const char *TAG = "anpr_main";

typedef struct {
    uint8_t *jpeg_buf; 
    size_t   jpeg_len;
    int64_t  timestamp_us;
} capture_msg_t;

static QueueHandle_t s_upload_queue;

//LiDAR trigger task (Core 0)

static void capture_and_queue(const char *reason)
{
    camera_set_capture_res();
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return;

    uint8_t *copy = heap_caps_malloc(fb->len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (copy) {
        memcpy(copy, fb->buf, fb->len);
        capture_msg_t msg = { .jpeg_buf = copy, .jpeg_len = fb->len, .timestamp_us = esp_timer_get_time() };
        if (xQueueSend(s_upload_queue, &msg, 0) != pdTRUE) {
            sd_logger_save_frame(copy, fb->len, reason);
            heap_caps_free(copy);
        }
    }
    esp_camera_fb_return(fb);
    camera_set_motion_res();
}

static void lidar_trigger_task(void *arg)
{
    int64_t last_capture = 0;
    while (1) {
        uint16_t dist = lidar_read_distance();
        int64_t now = esp_timer_get_time();
        bool trigger = (dist > 0 && dist < CONFIG_ANPR_LIDAR_THRESHOLD_CM);
        bool fallback = (now - last_capture >= FALLBACK_INTERVAL_US);

        if ((trigger || fallback) && (now - last_capture >= (int64_t)CAPTURE_COOLDOWN_S * 1000000LL)) {
            ESP_LOGI(TAG, "Trigger: %s", trigger ? "LiDAR" : "Fallback");
            capture_and_queue(trigger ? "lidar" : "fallback");
            last_capture = now;
        }
        vTaskDelay(pdMS_TO_TICKS(LIDAR_CHECK_MS));
    }
}

//upload task (Core 1)

static void upload_task(void *arg)
{
    capture_msg_t msg;
    while (1) {
        if (xQueueReceive(s_upload_queue, &msg, pdMS_TO_TICKS(UPLOAD_POLL_MS)) == pdTRUE) {
            plate_result_t res;
            if (plate_uploader_send(msg.jpeg_buf, msg.jpeg_len, &res) == ESP_OK) {
                if (res.plate_found) {
                    ESP_LOGI(TAG, "Plate: [%s] (%.2f)", res.plate_text, res.confidence);
                    char buf[128];
                    snprintf(buf, sizeof(buf), "{\"plate\":\"%s\",\"conf\":%.2f}", res.plate_text, res.confidence);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    esc_mqtt_publish(1, 1, strlen(buf), (uint8_t *)buf);
                }
            } else {
                sd_logger_save_frame(msg.jpeg_buf, msg.jpeg_len, "upload_fail");
            }
            heap_caps_free(msg.jpeg_buf);
        }

        char path[128];
        if (sd_logger_get_pending(path, sizeof(path)) == ESP_OK) {
            plate_result_t res;
            if (plate_uploader_send_from_sd(path, &res) == ESP_OK && res.plate_found) {
                ESP_LOGI(TAG, "SD Retry Success: [%s]", res.plate_text);
            }
        }
    }
}

//ENTRY POINT
void anpr_start(void)
{
    vTaskDelay(pdMS_TO_TICKS(500));
    ESP_LOGI(TAG, "ANPR SmartConnect Node-starting");
    printf("[SHW][ANPR]: Task startup\n");

    //SD
    esp_err_t ret = sd_logger_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SD card unavailable-offline buffering disabled");
    }

    //camera
    ESP_ERROR_CHECK(camera_init());
    camera_set_motion_res();   //start at cheap res

    //LiDAR
    ESP_ERROR_CHECK(lidar_init());

    //queue intertask
    s_upload_queue = xQueueCreate(UPLOAD_QUEUE_LEN, sizeof(capture_msg_t));
    if (!s_upload_queue) {
        ESP_LOGE(TAG, "Failed to create upload queue-halting");
        return;
    }

    //tasks
    //lidar_trigger_task pinned to Core 0 (UART reads)
    //upload_task pinned to Core 1 (network stack)
    xTaskCreatePinnedToCore(
        lidar_trigger_task, "lidar_trig",
        TASK_ANPR_STACK, NULL,
        TASK_PRIORITY_HIGH,
        NULL, 0);

    xTaskCreatePinnedToCore(
        upload_task, "upload",
        TASK_UPLOAD_STACK, NULL,
        TASK_PRIORITY_LOW,
        NULL, 1);

    ESP_LOGI(TAG, "ANPR tasks started");
}
