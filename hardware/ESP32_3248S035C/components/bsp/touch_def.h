// Touch definitions for ESP32_3248S035C
// Touch driver GT911 (I2C)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "GT911"

#include "gt911.h"

// Touch configuration
const gt911_config_t gt911_cfg = {
    .i2c_clk_speed = 400*1000,
    .rst_pin = 25, // GPIO 25
#if WITH_GT911_INT
    .int_pin = 21, // GPIO 21
#else
    .int_pin = -1, // INT pin not connected (by default)
#endif
    .swap_xy = true,
    .invert_x = true,
    .invert_y = false,    
};

#ifdef __cplusplus
} /* extern "C" */
#endif
