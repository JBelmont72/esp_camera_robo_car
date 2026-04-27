// main/camera_pins.h
#pragma once

/*
 * ELEGOO ESP32-WROVER Camera V1.5 / OV2640
 *
 * This board is the ESP32-WROVER camera module supplied with the
 * ELEGOO Smart Robot Car Kit V4 camera package. It is not the
 * ESP32-S3-EYE and it is not the AI-Thinker ESP32-CAM pin map.
 *
 * The 4-pin VCC/GND/TX/RX connector is not used for this project.
 * Programming/power is through the USB-C connector.
 */

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     21
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       19
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM        5
#define Y2_GPIO_NUM        4

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Optional onboard/status LED on many WROVER camera boards.
// Not used by the streaming-only test program.
#define CAM_PIN_LED        2

// Backward-compatible aliases used by older local files.
#define CAM_PIN_PWDN       PWDN_GPIO_NUM
#define CAM_PIN_RESET      RESET_GPIO_NUM
#define CAM_PIN_XCLK       XCLK_GPIO_NUM
#define CAM_PIN_SIOD       SIOD_GPIO_NUM
#define CAM_PIN_SIOC       SIOC_GPIO_NUM
#define CAM_PIN_D7         Y9_GPIO_NUM
#define CAM_PIN_D6         Y8_GPIO_NUM
#define CAM_PIN_D5         Y7_GPIO_NUM
#define CAM_PIN_D4         Y6_GPIO_NUM
#define CAM_PIN_D3         Y5_GPIO_NUM
#define CAM_PIN_D2         Y4_GPIO_NUM
#define CAM_PIN_D1         Y3_GPIO_NUM
#define CAM_PIN_D0         Y2_GPIO_NUM
#define CAM_PIN_VSYNC      VSYNC_GPIO_NUM
#define CAM_PIN_HREF       HREF_GPIO_NUM
#define CAM_PIN_PCLK       PCLK_GPIO_NUM
