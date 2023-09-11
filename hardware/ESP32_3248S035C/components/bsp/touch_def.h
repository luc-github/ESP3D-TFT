// Touch definitions for ESP32_3248S035C
// Touch driver GT911 (I2C)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "GT911"

#include "gt911.h"

gt911_config_t gt911_cfg = {
    .i2c_clk_speed = 400*1000,
    .rst_pin = 25, // GPIO 25
    .int_pin = -1, // INT pin not connected (by default)
    .swap_xy = true,
    .invert_x = true,
    .invert_y = false,    
};

#ifdef __cplusplus
} /* extern "C" */
#endif
