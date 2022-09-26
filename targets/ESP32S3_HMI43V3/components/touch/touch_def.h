//Pins definition for ESP32-S3 HMI43V3
//Touch driver FT5x06 I2C
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_LV_TOUCH_CONTROLLER_FT5x06 1
#define ENABLE_TOUCH_INPUT  1

#define FT5x06_I2C_PORT 1
#define FT5x06_ADDRESS (0x38)
#define FT5x06_TOUCH_MAX_POINTS 5
#define FT5x06_X_MIN           0
#define FT5x06_Y_MIN           0
#define FT5x06_X_MAX           1900
#define FT5x06_Y_MAX           1900
#define FT5x06_X_INV           0
#define FT5x06_Y_INV           0
#define FT5x06_XY_SWAP		   0

#ifdef __cplusplus
} /* extern "C" */
#endif