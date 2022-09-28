//Pins definition for ESP32S3_HMI43V3
//Touch driver FT5X06 I2C
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "FT5X06"

#define FT5x06_ADDR            (0x38)
#define FT5x06_CLK_SPEED       (400000)

#ifdef __cplusplus
} /* extern "C" */
#endif