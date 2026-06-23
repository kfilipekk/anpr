idf_component_register(
    SRCS "camera.c" "lidar.c" "main.c" "plate_uploader.c" "sd_card_logger.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp32-camera
        esp_driver_uart
        esp_driver_gpio
        heap
        esp_timer
        freertos
        nvs_flash
        esp_wifi
        esp_event
        lwip
        esp_http_client
        smartconnect_esp
        smartconnect_core_esp
)
