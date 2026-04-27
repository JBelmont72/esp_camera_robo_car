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
    esp_err_t err;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    err = camera_init_vga();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed in app_main: %s", esp_err_to_name(err));
        return;
    }
    // VGA this works great
    // ESP_ERROR_CHECK(camera_init_qvga()); // QVGA

    ESP_LOGI(TAG, "Initializing camera (VGA test)...");
    // ESP_ERROR_CHECK(camera_init_svga());

    ESP_LOGI(TAG, "Initializing camera...");

    ESP_LOGI(TAG, "Starting Wi-Fi...");
    ESP_ERROR_CHECK(wifi_init_sta("SpectrumSetup-41", "leastdinner914"));

    ESP_LOGI(TAG, "Starting HTTP stream...");
    ESP_ERROR_CHECK(http_stream_start());

    ESP_LOGI(TAG, "System ready");
}
