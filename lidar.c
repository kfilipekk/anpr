#include "lidar.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

static const char *TAG = "LIDAR";
#define LIDAR_UART_NUM UART_NUM_1
#define LIDAR_BAUD     115200
#define LIDAR_BUF_SIZE 512
#define LIDAR_TIMEOUT  50000
#define TF_LUNA_LEN    9
#define TF_LUNA_HDR    0x59

esp_err_t lidar_init(void)
{
    uart_config_t cfg = {
        .baud_rate = LIDAR_BAUD, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK(uart_driver_install(LIDAR_UART_NUM, LIDAR_BUF_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(LIDAR_UART_NUM, &cfg));
    return uart_set_pin(LIDAR_UART_NUM, CONFIG_ANPR_LIDAR_TX_GPIO, CONFIG_ANPR_LIDAR_RX_GPIO, -1, -1);
}

uint16_t lidar_read_distance(void)
{
    uint8_t buf[TF_LUNA_LEN];
    int64_t end = esp_timer_get_time() + LIDAR_TIMEOUT;
    while (esp_timer_get_time() < end) {
        uint8_t b;
        if (uart_read_bytes(LIDAR_UART_NUM, &b, 1, pdMS_TO_TICKS(5)) != 1) continue;
        
        static int pos = 0;
        if (pos < 2) {
            pos = (b == TF_LUNA_HDR) ? pos + 1 : 0;
        } else {
            buf[pos++] = b;
            if (pos == TF_LUNA_LEN) {
                pos = 0;
                uint8_t csum = 0;
                for (int i = 0; i < TF_LUNA_LEN - 1; i++) csum += buf[i];
                if (csum != buf[TF_LUNA_LEN - 1]) continue;

                uint16_t dist = buf[2] | (buf[3] << 8);
                uint16_t amp = buf[4] | (buf[5] << 8);
                if (amp < CONFIG_ANPR_LIDAR_MIN_AMPLITUDE || amp == 0xFFFF) return 0;
                return dist;
            }
        }
    }
    return 0;
}
