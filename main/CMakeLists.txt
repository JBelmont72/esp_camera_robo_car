idf_component_register(
    SRCS
        "main.c"
        "camera_init.c"
        "wifi_init.c"
        "http_stream.c"
    INCLUDE_DIRS
        "."
    REQUIRES esp_netif esp_wifi esp_event nvs_flash esp_system log esp_http_server esp32-camera
    PRIV_REQUIRES esp_driver_gpio
)
