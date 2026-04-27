#include "camera_init.h"
#include "camera_pins.h"

#include "esp_camera.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CAMERA";

static esp_err_t camera_init_common(framesize_t frame_size, int jpeg_quality)
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

    config.pin_pwdn  = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    // OV2640 works best at 20MHz on WROVER boards
    config.xclk_freq_hz = 20000000;

    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size   = frame_size;
    config.jpeg_quality = jpeg_quality;

    config.fb_count    = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode   = CAMERA_GRAB_LATEST;

    gpio_set_pull_mode(SIOD_GPIO_NUM, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SIOC_GPIO_NUM, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG, "Initializing camera");

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(err));
        return err;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Sensor not found");
        return ESP_FAIL;
    }

    // Basic stable defaults
    s->set_whitebal(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_gain_ctrl(s, 1);

    s->set_brightness(s, 1);
    s->set_contrast(s, 1);
    s->set_saturation(s, 1);

    s->set_pixformat(s, PIXFORMAT_JPEG);
    s->set_framesize(s, frame_size);
    s->set_quality(s, jpeg_quality);

    // Warm-up frames
    for (int i = 0; i < 5; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(30));
    }

    return ESP_OK;
}

esp_err_t camera_init_vga(void)
{
    return camera_init_common(FRAMESIZE_VGA, 12);
}

esp_err_t camera_init_qvga(void)
{
    return camera_init_common(FRAMESIZE_QVGA, 14);
}

esp_err_t camera_init_svga(void)
{
    return camera_init_common(FRAMESIZE_SVGA, 16);
}
