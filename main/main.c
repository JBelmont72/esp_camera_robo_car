/*
 * esp_camera_robo_car
 *
 * Camera streaming application for the Elegoo Smart Robot Car V4.0.
 * Uses the ESP32-S3 camera module (ESP32-WROVER-Camera / ESP32-S3-Camera board)
 * with the pin mapping documented by Elegoo community resources.
 *
 * Streams MJPEG video over Wi-Fi via a lightweight HTTP server.
 * Connect to the configured SSID and navigate to http://<device-ip>/stream
 * to view the live feed.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_camera.h"
#include "driver/gpio.h"

static const char *TAG = "elegoo_cam";

/* ---------------------------------------------------------------------------
 * Wi-Fi credentials – set via menuconfig or replace the defaults below.
 * Navigate to: Component config -> Elegoo Camera Config
 * --------------------------------------------------------------------------- */
#ifndef CONFIG_WIFI_SSID
#define CONFIG_WIFI_SSID "your_ssid"
#endif
#ifndef CONFIG_WIFI_PASSWORD
#define CONFIG_WIFI_PASSWORD "your_password"
#endif

/* ---------------------------------------------------------------------------
 * Elegoo Smart Robot Car V4.0 – ESP32-S3 Camera Pin Mapping
 *
 * Source: Elegoo official camera source code and community documentation.
 * --------------------------------------------------------------------------- */
#define CAM_PIN_PWDN    32   /* Power-down; pull high to disable sensor        */
#define CAM_PIN_RESET   -1   /* Hardware reset not connected; use software reset */
#define CAM_PIN_XCLK     0   /* External clock output to camera sensor          */
#define CAM_PIN_SIOD    26   /* I2C SDA (SCCB data)                             */
#define CAM_PIN_SIOC    27   /* I2C SCL (SCCB clock)                            */

#define CAM_PIN_D7      35   /* Parallel data bit 7 (MSB)                       */
#define CAM_PIN_D6      34   /* Parallel data bit 6                             */
#define CAM_PIN_D5      39   /* Parallel data bit 5                             */
#define CAM_PIN_D4      36   /* Parallel data bit 4                             */
#define CAM_PIN_D3      21   /* Parallel data bit 3                             */
#define CAM_PIN_D2      19   /* Parallel data bit 2                             */
#define CAM_PIN_D1      18   /* Parallel data bit 1                             */
#define CAM_PIN_D0       5   /* Parallel data bit 0 (LSB)                       */

#define CAM_PIN_VSYNC   25   /* Vertical sync                                   */
#define CAM_PIN_HREF    23   /* Horizontal reference                            */
#define CAM_PIN_PCLK    22   /* Pixel clock                                     */

/* Onboard flash LED (active-high on the Elegoo camera board) */
#define FLASH_GPIO_NUM   4

/* XCLK frequency recommended for stable streaming on this module */
#define XCLK_FREQ_HZ    20000000   /* 20 MHz */

/* ---------------------------------------------------------------------------
 * MJPEG streaming boundary
 * --------------------------------------------------------------------------- */
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART =
    "Content-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n";

/* ---------------------------------------------------------------------------
 * Camera initialisation
 * --------------------------------------------------------------------------- */
static esp_err_t camera_init(void)
{
    camera_config_t config = {
        .pin_pwdn      = CAM_PIN_PWDN,
        .pin_reset     = CAM_PIN_RESET,
        .pin_xclk      = CAM_PIN_XCLK,
        .pin_sccb_sda  = CAM_PIN_SIOD,
        .pin_sccb_scl  = CAM_PIN_SIOC,

        .pin_d7        = CAM_PIN_D7,
        .pin_d6        = CAM_PIN_D6,
        .pin_d5        = CAM_PIN_D5,
        .pin_d4        = CAM_PIN_D4,
        .pin_d3        = CAM_PIN_D3,
        .pin_d2        = CAM_PIN_D2,
        .pin_d1        = CAM_PIN_D1,
        .pin_d0        = CAM_PIN_D0,

        .pin_vsync     = CAM_PIN_VSYNC,
        .pin_href      = CAM_PIN_HREF,
        .pin_pclk      = CAM_PIN_PCLK,

        .xclk_freq_hz  = XCLK_FREQ_HZ,
        .ledc_timer    = LEDC_TIMER_0,
        .ledc_channel  = LEDC_CHANNEL_0,

        /* JPEG output avoids heavy YUV conversion on the MCU side */
        .pixel_format  = PIXFORMAT_JPEG,

        /* VGA (640×480) – requires PSRAM; see sdkconfig.defaults            */
        .frame_size    = FRAMESIZE_VGA,

        /* Quality: 0 (best) – 63 (worst); 10 is a good streaming trade-off */
        .jpeg_quality  = 10,

        /* Two frame buffers allow double-buffering for smoother streaming   */
        .fb_count      = 2,

        /* Store frame buffers in PSRAM                                       */
        .fb_location   = CAMERA_FB_IN_PSRAM,

        .grab_mode     = CAMERA_GRAB_WHEN_EMPTY,
    };

    ESP_LOGI(TAG, "Initialising camera (XCLK=%d Hz)", config.xclk_freq_hz);

    esp_err_t ret = esp_camera_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", ret);
    } else {
        ESP_LOGI(TAG, "Camera ready");
    }
    return ret;
}

/* ---------------------------------------------------------------------------
 * HTTP handler: /stream  – delivers an endless MJPEG stream
 * --------------------------------------------------------------------------- */
static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb        = NULL;
    esp_err_t    res       = ESP_OK;
    char         part_buf[128];

    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }

        /* Send the multipart boundary */
        res = httpd_resp_send_chunk(req, STREAM_BOUNDARY,
                                    strlen(STREAM_BOUNDARY));
        if (res != ESP_OK) {
            esp_camera_fb_return(fb);
            break;
        }

        /* Send the part header with the JPEG content length */
        size_t hlen = snprintf(part_buf, sizeof(part_buf),
                               STREAM_PART, fb->len);
        res = httpd_resp_send_chunk(req, part_buf, hlen);
        if (res != ESP_OK) {
            esp_camera_fb_return(fb);
            break;
        }

        /* Send the JPEG payload */
        res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        esp_camera_fb_return(fb);
        if (res != ESP_OK) {
            break;
        }
    }

    return res;
}

/* ---------------------------------------------------------------------------
 * HTTP handler: /  – minimal status page
 * --------------------------------------------------------------------------- */
static esp_err_t index_handler(httpd_req_t *req)
{
    const char *html =
        "<html><body>"
        "<h2>Elegoo ESP32-S3 Camera</h2>"
        "<p><a href=\"/stream\">Live Stream (MJPEG)</a></p>"
        "</body></html>";
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}

/* ---------------------------------------------------------------------------
 * Start the HTTP server and register URI handlers
 * --------------------------------------------------------------------------- */
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port    = 80;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return NULL;
    }

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL,
    };
    httpd_register_uri_handler(server, &index_uri);

    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL,
    };
    httpd_register_uri_handler(server, &stream_uri);

    ESP_LOGI(TAG, "HTTP server started on port %d", config.server_port);
    return server;
}

/* ---------------------------------------------------------------------------
 * Wi-Fi event handler
 * --------------------------------------------------------------------------- */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected – retrying...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connected. IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Stream URL: http://" IPSTR "/stream",
                 IP2STR(&event->ip_info.ip));
    }
}

/* ---------------------------------------------------------------------------
 * Wi-Fi station initialisation
 * --------------------------------------------------------------------------- */
static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi STA initialised. Connecting to SSID: %s",
             CONFIG_WIFI_SSID);
}

/* ---------------------------------------------------------------------------
 * Flash LED helper – brief blink on startup to confirm firmware is running
 * --------------------------------------------------------------------------- */
static void flash_led_blink(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << FLASH_GPIO_NUM),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_set_level(FLASH_GPIO_NUM, 1);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(FLASH_GPIO_NUM, 0);
}

/* ---------------------------------------------------------------------------
 * Application entry point
 * --------------------------------------------------------------------------- */
void app_main(void)
{
    /* Brief LED blink to signal firmware start */
    flash_led_blink();

    /* Initialise non-volatile storage (required by Wi-Fi driver) */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialise the camera with Elegoo pin mapping */
    ESP_ERROR_CHECK(camera_init());

    /* Connect to Wi-Fi */
    wifi_init_sta();

    /* Start the MJPEG streaming server */
    start_webserver();
}
