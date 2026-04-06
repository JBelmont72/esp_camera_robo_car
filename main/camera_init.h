
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize camera in QVGA (320x240) JPEG mode
 */
//esp_err_t camera_init_qvga(void);

/**
 * @brief Initialize camera in VGA (640x480) JPEG mode
 */
esp_err_t camera_init_vga(void);

// uncomment to select the vga mode desired
//esp_err_t camera_init_svga(void);
/**
 * @brief Deinitialize camera (optional but recommended when switching modes)
 */
void camera_deinit(void);

#ifdef __cplusplus
}
#endif



/*
//  top is camera_init.h for QVGA.  bottom for VGA





#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize ESP32-S3-EYE camera in QVGA (320x240) mode
esp_err_t camera_init_custom(void);

#ifdef __cplusplus
}
#endif





// worked with the working VGS camera_init.c for VGA

#pragma once
#include "esp_err.h"

esp_err_t camera_init_custom(void);
*/