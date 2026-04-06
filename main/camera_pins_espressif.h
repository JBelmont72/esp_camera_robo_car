
// main/camera_pins.h
#pragma once

//
// Pin definitions for ESP32-S3-EYE v2.2
//

#define CAM_PIN_PWDN    -1   // No PWDN pin
#define CAM_PIN_RESET   -1   // No RESET pin (software reset)

#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11

#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

// Aliases compatible with esp-camera examples
#define Y2_GPIO_NUM     CAM_PIN_D0
#define Y3_GPIO_NUM     CAM_PIN_D1
#define Y4_GPIO_NUM     CAM_PIN_D2
#define Y5_GPIO_NUM     CAM_PIN_D3
#define Y6_GPIO_NUM     CAM_PIN_D4
#define Y7_GPIO_NUM     CAM_PIN_D5
#define Y8_GPIO_NUM     CAM_PIN_D6
#define Y9_GPIO_NUM     CAM_PIN_D7

#define XCLK_GPIO_NUM   CAM_PIN_XCLK
#define PCLK_GPIO_NUM   CAM_PIN_PCLK
#define VSYNC_GPIO_NUM  CAM_PIN_VSYNC
#define HREF_GPIO_NUM   CAM_PIN_HREF
#define SIOD_GPIO_NUM   CAM_PIN_SIOD
#define SIOC_GPIO_NUM   CAM_PIN_SIOC
#define PWDN_GPIO_NUM   CAM_PIN_PWDN
#define RESET_GPIO_NUM  CAM_PIN_RESET


/*
// main/camera_pins.h
#pragma once

//
// Correct pin definitions for ESP32-S3-EYE v2.2
// Matches Espressif schematic + working IDF examples
//

#define CAM_PIN_PWDN    -1   // No PWDN pin
#define CAM_PIN_RESET   -1   // No RESET pin (software reset)

#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11

#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

//
// Optional aliases (maintains compatibility with esp-camera examples)
//

#define Y2_GPIO_NUM     CAM_PIN_D0
#define Y3_GPIO_NUM     CAM_PIN_D1
#define Y4_GPIO_NUM     CAM_PIN_D2
#define Y5_GPIO_NUM     CAM_PIN_D3
#define Y6_GPIO_NUM     CAM_PIN_D4
#define Y7_GPIO_NUM     CAM_PIN_D5
#define Y8_GPIO_NUM     CAM_PIN_D6
#define Y9_GPIO_NUM     CAM_PIN_D7

#define XCLK_GPIO_NUM   CAM_PIN_XCLK
#define PCLK_GPIO_NUM   CAM_PIN_PCLK
#define VSYNC_GPIO_NUM  CAM_PIN_VSYNC
#define HREF_GPIO_NUM   CAM_PIN_HREF
#define SIOD_GPIO_NUM   CAM_PIN_SIOD
#define SIOC_GPIO_NUM   CAM_PIN_SIOC
#define PWDN_GPIO_NUM   CAM_PIN_PWDN
#define RESET_GPIO_NUM  CAM_PIN_RESET
*/