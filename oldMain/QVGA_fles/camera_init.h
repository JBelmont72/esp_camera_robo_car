


//  top is camera_init.h for QVGA.  bottom for VGA
/*
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
*/




// worked with the working VGA camera_init.c for VGA

#pragma once
#include "esp_err.h"

esp_err_t camera_init_custom(void);
