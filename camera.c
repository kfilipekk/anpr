#include "camera.h"

#include "esp_camera.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "sdkconfig.h"

#include <string.h>

static const char *TAG = "CAMERA";


#define CAM_PIN_PWDN    (-1)
#define CAM_PIN_RESET   (-1)
#define CAM_PIN_XCLK      10  //master clock output to OV2640
#define CAM_PIN_SIOD      40  //SCCB data  (like I2C SDA)
#define CAM_PIN_SIOC      39  //SCCB clock (like I2C SCL)
#define CAM_PIN_D7        48  //DVP data bit 9 (MSB)
#define CAM_PIN_D6        11  //DVP data bit 8
#define CAM_PIN_D5        12  //DVP data bit 7
#define CAM_PIN_D4        14  //DVP data bit 6
#define CAM_PIN_D3        16  //DVP data bit 5
#define CAM_PIN_D2        18  //DVP data bit 4
#define CAM_PIN_D1        17  //DVP data bit 3
#define CAM_PIN_D0        15  //DVP data bit 2 (LSB)
#define CAM_PIN_VSYNC     38  //vertical sync
#define CAM_PIN_HREF      47  //horizontal reference
#define CAM_PIN_PCLK      13  //pixel clock


#define CAM_XCLK_FREQ   20000000
#define CAM_FB_COUNT    2

esp_err_t camera_init(void)
{
    camera_config_t config = {
        .pin_pwdn     = CAM_PIN_PWDN,
        .pin_reset    = CAM_PIN_RESET,
        .pin_xclk     = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,
        .pin_d7       = CAM_PIN_D7,
        .pin_d6       = CAM_PIN_D6,
        .pin_d5       = CAM_PIN_D5,
        .pin_d4       = CAM_PIN_D4,
        .pin_d3       = CAM_PIN_D3,
        .pin_d2       = CAM_PIN_D2,
        .pin_d1       = CAM_PIN_D1,
        .pin_d0       = CAM_PIN_D0,
        .pin_vsync    = CAM_PIN_VSYNC,
        .pin_href     = CAM_PIN_HREF,
        .pin_pclk     = CAM_PIN_PCLK,

        .xclk_freq_hz = CAM_XCLK_FREQ,
        .ledc_timer   = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size   = FRAMESIZE_SVGA,
        .jpeg_quality = CONFIG_ANPR_CAMERA_JPEG_QUALITY,
        .fb_count     = CAM_FB_COUNT,
        .fb_location  = CAMERA_FB_IN_PSRAM,
        .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
    };

    esp_err_t ret = esp_camera_init(&config);
    if (ret != ESP_OK) return ret;

    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 0);
        s->set_contrast(s, 1);
        s->set_saturation(s, 0);
        s->set_whitebal(s, 1);
        s->set_gain_ctrl(s, 1);
    }

    ESP_LOGI(TAG, "Camera ready (quality %d)", CONFIG_ANPR_CAMERA_JPEG_QUALITY);
    return ESP_OK;
}

void camera_set_motion_res(void)
{
    sensor_t *s = esp_camera_sensor_get();
    if (s) s->set_framesize(s, FRAMESIZE_QVGA);
}

void camera_set_capture_res(void)
{
    sensor_t *s = esp_camera_sensor_get();
    if (s) s->set_framesize(s, FRAMESIZE_SVGA);
}
