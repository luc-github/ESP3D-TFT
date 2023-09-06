//Pins definition for ESP32S3_BZM_TFT35_GT911
//Touch driver FT5X06 I2C
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "GT911"

#define GT911_ADDR            (0x5D)
#define GT911_CLK_SPEED       (400000)
//#define GT911_TOUCH_IRQ       (-1) //Not connected
//#define GT911_TOUCH_IRQ_PRESS 0 // not working currently - FIX ME
#define GT911_TOUCH_PRESS 1
#define GT911_RESET_PIN (41)

#ifdef __cplusplus
} /* extern "C" */
#endif