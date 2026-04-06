

//  camera_init.c for QVGA
#include "esp_camera.h"
#include "camera_pins.h"
#include "camera_init.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CAMERA";

esp_err_t camera_init_custom(void)
{
    camera_config_t config = {0};

    // LEDC (XCLK generator)
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;

    // Camera data pins
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;

    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    // Power & reset
    config.pin_pwdn  = -1;
    config.pin_reset = -1;

    // Camera timing & format
    config.xclk_freq_hz = 8000000;     // Lower XCLK → more stable JPEG
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size   = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 12;             // Reasonable quality other recommendation was go to 14 to get rid of error
    config.fb_count     = 1;              // Two frame buffers
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.grab_mode    = CAMERA_GRAB_LATEST;

    // Pull-ups for SCCB
    gpio_set_pull_mode(SIOD_GPIO_NUM, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SIOC_GPIO_NUM, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG, "Initializing OV2640 camera (QVGA)...");

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", err);
        return err;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }

    // Auto controls
    s->set_whitebal(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 1);
    s->set_ae_level(s, 0);
    s->set_gain_ctrl(s, 1);

    // Image tuning
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_gainceiling(s, GAINCEILING_2X);

    // Safety
    s->set_pixformat(s, PIXFORMAT_JPEG);
    s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_quality(s, 12);

    // Warm-up frames
    for (int i = 0; i < 10; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            esp_camera_fb_return(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }

    ESP_LOGI(TAG, "Camera initialized at QVGA (320x240)");
    return ESP_OK;
}

