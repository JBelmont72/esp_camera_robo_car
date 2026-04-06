#include "wifi_init.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"

static const char *TAG = "WIFI";

// --------------------------------------------------
// Event Handler (ESP-IDF 6.1 Compliant)
// --------------------------------------------------
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {

        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WiFi started, connecting...");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGW(TAG, "WiFi disconnected — reconnecting…");
            esp_wifi_connect();
            break;

        default:
            break;
        }
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        char ip[16], gw[16], nm[16];

        esp_ip4addr_ntoa(&event->ip_info.ip, ip, sizeof(ip));
        esp_ip4addr_ntoa(&event->ip_info.gw, gw, sizeof(gw));
        esp_ip4addr_ntoa(&event->ip_info.netmask, nm, sizeof(nm));

        ESP_LOGI(TAG, "Got IP:      %s", ip);
        ESP_LOGI(TAG, "Gateway:     %s", gw);
        ESP_LOGI(TAG, "Netmask:     %s", nm);
    }
}

// --------------------------------------------------
// Wi-Fi Initialization (Perfect for ESP-IDF 6.1)
// --------------------------------------------------
esp_err_t wifi_init_sta(const char *ssid, const char *pass)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_netif_init());

    // Safe: Do NOT crash if already created
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Event loop creation failed: %s", esp_err_to_name(err));
        return err;
    }


    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register all Wi-Fi & IP events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               &wifi_event_handler,
                                               NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler,
                                               NULL));

    // Standard STA configuration
    wifi_config_t wifi_config = { 0 };
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, pass);

    // Ensure WPA2/3 automatic mode (IDF 6.x requirement)
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());


    /* Disable Wi-Fi power save to prevent camera corruption added 17 ded */
    esp_wifi_set_ps(WIFI_PS_NONE);



    ESP_LOGI(TAG, "WiFi init complete");
    return ESP_OK;
}

