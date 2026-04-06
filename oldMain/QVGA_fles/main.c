
/* main.c*/

#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "camera_init.h"
#include "wifi_init.h"
#include "http_stream.h"

static const char *TAG = "MAIN_APP";

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Initializing camera...");
    ESP_ERROR_CHECK(camera_init_custom());

    ESP_LOGI(TAG, "Starting Wi-Fi...");
    ESP_ERROR_CHECK(wifi_init_sta("NETGEAR48-2.4-LOFT", "waterypanda901"));
    

    ESP_LOGI(TAG, "Starting HTTP stream...");
    ESP_ERROR_CHECK(http_stream_start());

    ESP_LOGI(TAG, "System ready");
}



/* ESP_ERROR_CHECK(wifi_init_sta("SpectrumSetup-41", "leastdinner914"));*/

