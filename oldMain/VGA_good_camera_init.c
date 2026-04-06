

/* the first version is from 17 dec and works great but have to use VGA to avoid the nocal errors.  this chat https://chatgpt.com/c/694325b1-1fd0-8330-beb0-0483211ade5f has the explanaiton of why and also my obsidian file
file name=.  esp32s3 repair image size and no cal jpeg error
*/
#include "camera_init.h"
#include "camera_pins.h"
#include "esp_camera.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CAMERA";

esp_err_t camera_init_custom(void)
{
    camera_config_t config = {0};

    /*----------------------------------------------------
     * LEDC (XCLK generator)
     *----------------------------------------------------*/
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;

    /*----------------------------------------------------
     * Camera data pins (OV2640 → ESP32-S3-EYE)
     *----------------------------------------------------*/
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

    /*----------------------------------------------------
     * Power & reset (not used on ESP32-S3-EYE)
     *----------------------------------------------------*/
    config.pin_pwdn  = -1;
    config.pin_reset = -1;

    /*----------------------------------------------------
     * Camera timing & format
     *----------------------------------------------------*/

    /* 🔑 LOWER XCLK = MUCH more stable JPEG on S3 */
    config.xclk_freq_hz = 8000000;

    /* OV2640 supports JPEG natively */
    config.pixel_format = PIXFORMAT_JPEG;

    /* 🔑 Allocate buffers LARGE ENOUGH
       You may DOWNscale later, but NEVER upscale */
    // config.frame_size   = FRAMESIZE_VGA;   // 640x480
    config.frame_size   = FRAMESIZE_VGA;   // 320 x 240
    // config.frame_size = FRAMESIZE_SVGA;  // 800x600 (optional)

    /* JPEG quality: lower = better quality, larger size i reduced it from 12 to 14 */
    config.jpeg_quality = 14;

    /*----------------------------------------------------
     * Frame buffer configuration
     *----------------------------------------------------*/

    /* 2 buffers = best stability for streaming */
    config.fb_count     = 2;

    /* OV2640 + JPEG + VGA requires PSRAM */
    config.fb_location  = CAMERA_FB_IN_PSRAM;

    /* Always grab the most recent frame */
    config.grab_mode    = CAMERA_GRAB_LATEST;

    /*----------------------------------------------------
     * REQUIRED pullups for OV2640 SCCB
     *----------------------------------------------------*/
    gpio_set_pull_mode(SIOD_GPIO_NUM, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SIOC_GPIO_NUM, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG, "Initializing OV2640 camera...");

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", err);
        return err;
    }

    /*----------------------------------------------------
     * Sensor tuning (applied AFTER init)
     *----------------------------------------------------*/
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }

    /*----------------------------------------------------
     * Auto controls (recommended ON)
     *----------------------------------------------------*/
    s->set_whitebal(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 1);
    s->set_ae_level(s, 0);
    s->set_gain_ctrl(s, 1);

    /*----------------------------------------------------
     * Image tuning (safe values)
     *----------------------------------------------------*/
    s->set_brightness(s, 1);     // -2 .. +2
    s->set_contrast(s, 2);       // -2 .. +2
    s->set_saturation(s, 1);     // -2 .. +2

    /*----------------------------------------------------
     * Gain & noise control
     *----------------------------------------------------*/
    s->set_gainceiling(s, GAINCEILING_2X);

    /*----------------------------------------------------
     * 🔑 FINAL SAFETY FORCE
     * Ensures sensor matches allocated buffers
     *----------------------------------------------------*/
    s->set_pixformat(s, PIXFORMAT_JPEG);
    s->set_framesize(s, FRAMESIZE_VGA);   // MUST ≤ config.frame_size
    s->set_quality(s, 12);

    /*----------------------------------------------------
    * 🔑 DISCARD INITIAL FRAMES (OV2640 WARM-UP)
    *----------------------------------------------------*/
    for (int i = 0; i < 10; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            esp_camera_fb_return(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(30));
}



    /*----------------------------------------------------
     * Log final resolution for verification
     *----------------------------------------------------*/
    ESP_LOGI(TAG, "Camera initialized at VGA (640x480)");

    return ESP_OK;
}






/*
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

    config.xclk_freq_hz = 8000000;                   // Current
    config.pixel_format = PIXFORMAT_JPEG;              // Current

    // config.frame_size   = FRAMESIZE_QVGA;             // Current
    // Recommended resolutions to try
    config.frame_size   = FRAMESIZE_VGA;            // 640x480
    // config.frame_size   = FRAMESIZE_SVGA;           // 800x600
    // config.frame_size   = FRAMESIZE_XGA;           // 800x600

    config.jpeg_quality = 12;                          // Current
    config.fb_count     = 2;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.grab_mode    = CAMERA_GRAB_LATEST;      // Current changed 17 dec from grab when empty
    // config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;     // Current

    
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

    // Current settings
    // s->set_brightness(s, 0);     // -2 to 2
    //s->set_contrast(s, 0);       // -2 to 2
    // s->set_saturation(s, 0);     // -2 to 2
    s->set_whitebal(s, 0);      // was 1 on 15 dec changed to 0
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 1);
    s->set_ae_level(s, 0);

    // Recommended adjustments 
    s->set_brightness(s, 2);     // Adjust brightness with 5  try 2
    s->set_contrast(s, 5);      // Adjust contrast and with 10 looked better but NO-EOI message,try 5
    s->set_saturation(s, 10);    // Increase saturation suggested was 50 i will start at 10(original was0)
    // s->set_gainceiling(s, GAINCEILING_4X); // Adjust gain ceiling

    //  FORCE JPEG MODE 
    s->set_pixformat(s, PIXFORMAT_JPEG);
    s->set_framesize(s, FRAMESIZE_VGA);               // Current
    // s->set_framesize(s, FRAMESIZE_XGA);               // try along with above designation 15 dec SVGA seemed to work well
    // s->set_framesize(s, FRAMESIZE_SVGA);               // try along with above designation 15 dec SVGA seemed to work well
    s->set_quality(s, 12);
    s->set_gainceiling(s, GAINCEILING_2X);             // Current

    ESP_LOGI(TAG, "OV2640 initialized successfully");
    return ESP_OK;
}
*/
