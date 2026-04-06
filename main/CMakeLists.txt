# my lib from camera_init.c for VGA:

#include "camera_init.h"
#include "camera_pins.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

# main/CMakeLists.txt for QVGA
idf_component_register(
    SRCS
        "main.c"
        "camera_init.c"
        "wifi_init.c"
        "http_stream.c"
    INCLUDE_DIRS
        "."        # include all headers in main/
    REQUIRES esp_netif esp_wifi esp_event nvs_flash esp_system log esp_http_server
    PRIV_REQUIRES esp_driver_gpio
)






# main/CMakeLists.txt for QVGA
# idf_component_register(
#     SRCS
#         "main.c"
#         "camera_init.c"
#         "wifi_init.c"
#         "http_stream.c"
#     INCLUDE_DIRS
#         "."        # include all headers in main/
#     REQUIRES esp_netif esp_wifi esp_event nvs_flash esp_system log esp_http_server
#     PRIV_REQUIRES esp_driver_gpio
# )

# Optionally, link extra components
# REQUIRES esp32 camera
