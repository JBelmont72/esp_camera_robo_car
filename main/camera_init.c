


#include "camera_init.h"
#include "camera_pins.h"

#include "esp_camera.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CAMERA";

/*----------------------------------------------------
 * my Common init function
 *----------------------------------------------------*/
static esp_err_t camera_init_common(framesize_t frame_size, int jpeg_quality)
{
    camera_config_t config = {0};

    /* LEDC (XCLK) */
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;

    /* Data pins */
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

    /* Power/reset not used */
    config.pin_pwdn  = -1;
    config.pin_reset = -1;

    /* Stable JPEG timing for ESP32-S3 can adjust to 10,000,000 */
    config.xclk_freq_hz = 8000000;

    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size   = frame_size;
    config.jpeg_quality = jpeg_quality;

    config.fb_count    = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode   = CAMERA_GRAB_LATEST;

    gpio_set_pull_mode(SIOD_GPIO_NUM, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SIOC_GPIO_NUM, GPIO_PULLUP_ONLY);
//  could use belowbut would just correct the log, see my ReadMe.md
/*
    const char *mode_str = "UNKNOWN";

    switch (frame_size) {
        case FRAMESIZE_QVGA: mode_str = "QVGA"; break;
        case FRAMESIZE_VGA:  mode_str = "VGA";  break;
        case FRAMESIZE_SVGA: mode_str = "SVGA"; break;
    }

    ESP_LOGI(TAG, "Initializing camera (%s)", mode_str);

*/


    ESP_LOGI(TAG, "Initializing camera (%s)",
             frame_size == FRAMESIZE_VGA ? "VGA" : "QVGA");

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

    /* Auto controls */
    s->set_whitebal(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 1);
    s->set_gain_ctrl(s, 1);
    s->set_ae_level(s, 0);

    /* Image tuning */
    s->set_brightness(s, 1);
    s->set_contrast(s, 2);
    s->set_saturation(s, 1);

    s->set_gainceiling(s, GAINCEILING_2X);

    /*  Final enforce (MATCH config!) */
    s->set_pixformat(s, PIXFORMAT_JPEG);
    s->set_framesize(s, frame_size);
    s->set_quality(s, jpeg_quality);

    /* OV2640 warm-up discard */
    for (int i = 0; i < 10; i++) {
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
    // if use SVGA probably needs lower quality (higher number)
    return camera_init_common(FRAMESIZE_SVGA, 16);
}






//  camera_init.c for QVGA below
/*
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

*/


/*

// File: camera_init.c
#include "esp_err.h"        // esp_err_t, ESP_OK, ESP_FAIL
#include "esp_log.h"        // ESP_LOGI, ESP_LOGE
#include "esp_camera.h"     // camera_config_t, esp_camera_* functions
#include "driver/ledc.h"    // LEDC_CHANNEL_0, LEDC_TIMER_0
#include "driver/gpio.h"    // gpio_set_pull_mode, GPIO_PULLUP_ONLY
#include "freertos/FreeRTOS.h" // FreeRTOS types
#include "freertos/task.h"  // vTaskDelay, pdMS_TO_TICKS

#include "camera_pins.h"    // ESP32-S3 Eye pin definitions

static const char *TAG = "CAMERA";

// QVGA initialization (240x240) for faster FPS
esp_err_t camera_init_custom_qvga(void)
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

    // No power-down or reset pins
    config.pin_pwdn  = -1;
    config.pin_reset = -1;

    // Camera timing & format
    config.xclk_freq_hz = 8000000; // lower XCLK = more stable JPEG
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size   = FRAMESIZE_QVGA; // 240x240 for faster FPS
    config.jpeg_quality = 12;

    // Frame buffer configuration
    config.fb_count    = 2;                  // double buffer
    config.fb_location = CAMERA_FB_IN_PSRAM; // use PSRAM
    config.grab_mode   = CAMERA_GRAB_LATEST;

    // Pull-ups for SCCB
    gpio_set_pull_mode(SIOD_GPIO_NUM, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SIOC_GPIO_NUM, GPIO_PULLUP_ONLY);

    ESP_LOGI(TAG,


*/






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







