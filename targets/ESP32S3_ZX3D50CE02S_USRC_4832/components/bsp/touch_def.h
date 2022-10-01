//Pins definition for ESP32S3_ZX3D50CE02S_USRC_4832
//Touch driver FT5X06 I2C
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "FT5X06"

#define FT5x06_ADDR            (0x38)
#define FT5x06_CLK_SPEED       (400000)
//#define FT5x06_TOUCH_IRQ       (7) //GPIO 7
//#define FT5x06_TOUCH_IRQ_PRESS 0 // not working currently - FIX ME
#define FT5x06_TOUCH_PRESS 1
#define FT5x06_RESET           (4) //GPIO 4 same as panel

#ifdef __cplusplus
} /* extern "C" */
#endif