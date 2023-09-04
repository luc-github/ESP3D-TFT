// Pins definition for ESP32_3248S035C
// Touch driver GT911 I2C
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "GT911"

#define GT911_ADDR        (0x5D)
#define GT911_CLK_SPEED   (400000)
// #define GT911_TOUCH_IRQ   (21)
// #define GT911_TOUCH_IRQ_PRESS 0 // not working currently - FIX ME
#define GT911_TOUCH_PRESS 1
#define GT911_RESET_PIN   (25)
#define GT911_HOR_RES_MAX (480)
#define GT911_VER_RES_MAX (320)
#define GT911_SWAP_XY     1
#define GT911_X_INV       1
#define GT911_Y_INV       0

#ifdef __cplusplus
} /* extern "C" */
#endif