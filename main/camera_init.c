#include "camera_init.h"
#include "camera_pins.h"

#include "esp_camera.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdbool.h>
#include <stdint.h>

static const char *TAG = "CAMERA";
static bool s_sccb_scan_done = false;

static esp_err_t sccb_probe_address(i2c_port_t port, uint8_t addr)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(30));
    i2c_cmd_link_delete(cmd);
    return err;
}

static void scan_sccb_pin_pairs(void)
{
    static const int candidate_pins[] = {26, 27, 21, 22, 23, 25, 32, 33, 4, 5, 18, 19};
    static const uint8_t candidate_addrs[] = {0x21, 0x30, 0x31, 0x3c};

    ESP_LOGI(TAG, "SCCB scan: trying likely SDA/SCL pin pairs...");

    for (size_t pi = 0; pi < sizeof(candidate_pins) / sizeof(candidate_pins[0]); ++pi)
    {
        for (size_t pj = 0; pj < sizeof(candidate_pins) / sizeof(candidate_pins[0]); ++pj)
        {
            if (pi == pj)
            {
                continue;
            }

            int sda = candidate_pins[pi];
            int scl = candidate_pins[pj];

            for (int port = 0; port <= 1; ++port)
            {
                i2c_config_t conf = {
                    .mode = I2C_MODE_MASTER,
                    .sda_io_num = sda,
                    .scl_io_num = scl,
                    .sda_pullup_en = GPIO_PULLUP_ENABLE,
                    .scl_pullup_en = GPIO_PULLUP_ENABLE,
                    .master.clk_speed = 100000,
                    .clk_flags = 0,
                };

                if (i2c_param_config(port, &conf) != ESP_OK)
                {
                    continue;
                }
                if (i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0) != ESP_OK)
                {
                    continue;
                }

                for (size_t ai = 0; ai < sizeof(candidate_addrs) / sizeof(candidate_addrs[0]); ++ai)
                {
                    uint8_t addr = candidate_addrs[ai];
                    if (sccb_probe_address(port, addr) == ESP_OK)
                    {
                        ESP_LOGW(TAG, "SCCB candidate found: SDA=%d SCL=%d port=%d addr=0x%02x", sda, scl, port, addr);
                    }
                }

                i2c_driver_delete(port);
            }
        }
    }

    ESP_LOGI(TAG, "SCCB scan complete");
}

typedef struct
{
    const char *name;
    int pin_pwdn;
    int pin_reset;
    int pin_xclk;
    int pin_sccb_sda;
    int pin_sccb_scl;
    int pin_d7;
    int pin_d6;
    int pin_d5;
    int pin_d4;
    int pin_d3;
    int pin_d2;
    int pin_d1;
    int pin_d0;
    int pin_vsync;
    int pin_href;
    int pin_pclk;
} camera_pin_profile_t;

static const camera_pin_profile_t s_camera_profiles[] = {
    // Project default (current header)
    {
        .name = "WROVER_V1_5_DEFAULT",
        .pin_pwdn = PWDN_GPIO_NUM,
        .pin_reset = RESET_GPIO_NUM,
        .pin_xclk = XCLK_GPIO_NUM,
        .pin_sccb_sda = SIOD_GPIO_NUM,
        .pin_sccb_scl = SIOC_GPIO_NUM,
        .pin_d7 = Y9_GPIO_NUM,
        .pin_d6 = Y8_GPIO_NUM,
        .pin_d5 = Y7_GPIO_NUM,
        .pin_d4 = Y6_GPIO_NUM,
        .pin_d3 = Y5_GPIO_NUM,
        .pin_d2 = Y4_GPIO_NUM,
        .pin_d1 = Y3_GPIO_NUM,
        .pin_d0 = Y2_GPIO_NUM,
        .pin_vsync = VSYNC_GPIO_NUM,
        .pin_href = HREF_GPIO_NUM,
        .pin_pclk = PCLK_GPIO_NUM,
    },
    // Same mapping but with PWDN on GPIO32 (common on some clones)
    {
        .name = "WROVER_V1_5_PWDN32",
        .pin_pwdn = 32,
        .pin_reset = -1,
        .pin_xclk = 21,
        .pin_sccb_sda = 26,
        .pin_sccb_scl = 27,
        .pin_d7 = 35,
        .pin_d6 = 34,
        .pin_d5 = 39,
        .pin_d4 = 36,
        .pin_d3 = 19,
        .pin_d2 = 18,
        .pin_d1 = 5,
        .pin_d0 = 4,
        .pin_vsync = 25,
        .pin_href = 23,
        .pin_pclk = 22,
    },
    // AI-Thinker-compatible fallback profile
    {
        .name = "AI_THINKER_COMPAT",
        .pin_pwdn = 32,
        .pin_reset = -1,
        .pin_xclk = 0,
        .pin_sccb_sda = 26,
        .pin_sccb_scl = 27,
        .pin_d7 = 35,
        .pin_d6 = 34,
        .pin_d5 = 39,
        .pin_d4 = 36,
        .pin_d3 = 21,
        .pin_d2 = 19,
        .pin_d1 = 18,
        .pin_d0 = 5,
        .pin_vsync = 25,
        .pin_href = 23,
        .pin_pclk = 22,
    },
    // Wiring fallback where SCCB lines are reversed on clone boards
    {
        .name = "WROVER_V1_5_SCCB_SWAPPED",
        .pin_pwdn = -1,
        .pin_reset = -1,
        .pin_xclk = 21,
        .pin_sccb_sda = 27,
        .pin_sccb_scl = 26,
        .pin_d7 = 35,
        .pin_d6 = 34,
        .pin_d5 = 39,
        .pin_d4 = 36,
        .pin_d3 = 19,
        .pin_d2 = 18,
        .pin_d1 = 5,
        .pin_d0 = 4,
        .pin_vsync = 25,
        .pin_href = 23,
        .pin_pclk = 22,
    },
};

static const int s_xclk_candidates_hz[] = {8000000, 10000000, 20000000};
static const int s_sccb_port_candidates[] = {0, 1};

static esp_err_t camera_init_common(framesize_t frame_size, int jpeg_quality)
{
    esp_err_t last_err = ESP_FAIL;

    if (!s_sccb_scan_done)
    {
        scan_sccb_pin_pairs();
        s_sccb_scan_done = true;
    }

    for (size_t p = 0; p < sizeof(s_camera_profiles) / sizeof(s_camera_profiles[0]); ++p)
    {
        const camera_pin_profile_t *profile = &s_camera_profiles[p];

        for (size_t xc = 0; xc < sizeof(s_xclk_candidates_hz) / sizeof(s_xclk_candidates_hz[0]); ++xc)
        {
            const int xclk_hz = s_xclk_candidates_hz[xc];

            for (size_t pc = 0; pc < sizeof(s_sccb_port_candidates) / sizeof(s_sccb_port_candidates[0]); ++pc)
            {
                const int sccb_port = s_sccb_port_candidates[pc];

                camera_config_t config = {0};
                config.ledc_channel = LEDC_CHANNEL_0;
                config.ledc_timer = LEDC_TIMER_0;

                config.pin_d0 = profile->pin_d0;
                config.pin_d1 = profile->pin_d1;
                config.pin_d2 = profile->pin_d2;
                config.pin_d3 = profile->pin_d3;
                config.pin_d4 = profile->pin_d4;
                config.pin_d5 = profile->pin_d5;
                config.pin_d6 = profile->pin_d6;
                config.pin_d7 = profile->pin_d7;

                config.pin_xclk = profile->pin_xclk;
                config.pin_pclk = profile->pin_pclk;
                config.pin_vsync = profile->pin_vsync;
                config.pin_href = profile->pin_href;
                config.pin_sccb_sda = profile->pin_sccb_sda;
                config.pin_sccb_scl = profile->pin_sccb_scl;

                config.pin_pwdn = profile->pin_pwdn;
                config.pin_reset = profile->pin_reset;

                config.xclk_freq_hz = xclk_hz;
                config.pixel_format = PIXFORMAT_JPEG;
                config.frame_size = frame_size;
                config.jpeg_quality = jpeg_quality;
                config.fb_count = 2;
                config.fb_location = CAMERA_FB_IN_PSRAM;
                config.grab_mode = CAMERA_GRAB_LATEST;
                config.sccb_i2c_port = sccb_port;

                if (profile->pin_sccb_sda >= 0)
                {
                    gpio_set_pull_mode(profile->pin_sccb_sda, GPIO_PULLUP_ONLY);
                }
                if (profile->pin_sccb_scl >= 0)
                {
                    gpio_set_pull_mode(profile->pin_sccb_scl, GPIO_PULLUP_ONLY);
                }

                ESP_LOGI(TAG, "Initializing camera profile=%s xclk=%d sccb_port=%d", profile->name, xclk_hz, sccb_port);
                esp_err_t err = esp_camera_init(&config);
                if (err != ESP_OK)
                {
                    last_err = err;
                    ESP_LOGW(TAG, "Camera init failed for profile=%s xclk=%d sccb_port=%d: %s",
                             profile->name, xclk_hz, sccb_port, esp_err_to_name(err));
                    esp_camera_deinit();
                    continue;
                }

                sensor_t *s = esp_camera_sensor_get();
                if (!s)
                {
                    ESP_LOGW(TAG, "Sensor not found for profile=%s xclk=%d sccb_port=%d", profile->name, xclk_hz, sccb_port);
                    esp_camera_deinit();
                    last_err = ESP_FAIL;
                    continue;
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
                for (int i = 0; i < 5; i++)
                {
                    camera_fb_t *fb = esp_camera_fb_get();
                    if (fb)
                    {
                        esp_camera_fb_return(fb);
                    }
                    vTaskDelay(pdMS_TO_TICKS(30));
                }

                ESP_LOGI(TAG, "Camera initialized with profile=%s xclk=%d sccb_port=%d", profile->name, xclk_hz, sccb_port);
                return ESP_OK;
            }
        }
    }

    ESP_LOGE(TAG, "Camera init failed for all profiles");
    return last_err;
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
