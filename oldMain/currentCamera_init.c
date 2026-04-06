// copy of camera_init.c moving here for safe keeping 15 dec


#include "camera_init.h"
#include "camera_pins.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "CAMERA";

esp_err_t camera_init_custom(void)
{
    camera_config_t config = {0};

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;

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

    config.pin_pwdn  = -1;
    config.pin_reset = -1;

    config.xclk_freq_hz = 10000000;
    config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size   = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count     = 3;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
 /*   config.grab_mode    = CAMERA_GRAB_LATEST;*/
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

    /* REQUIRED for OV2640 on ESP32-S3 */
    gpio_set_pull_mode(SIOD_GPIO_NUM, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SIOC_GPIO_NUM, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG, "Initializing OV2640 camera...");
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

    s->set_brightness(s, 0);     // -2 to 2 added dec13 night
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_whitebal(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 1);
    s->set_ae_level(s, 0);


    /* 🔑 FORCE JPEG MODE */
    s->set_pixformat(s, PIXFORMAT_JPEG);
    s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_quality(s, 12);
    s->set_gainceiling(s, GAINCEILING_2X);




    ESP_LOGI(TAG, "OV2640 initialized successfully");
    return ESP_OK;
}


