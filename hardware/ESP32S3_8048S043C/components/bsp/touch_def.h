// Touch definitions for ESP32_8048S043C
// Touch driver GT911 (I2C)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "GT911"

#include "gt911.h"

const gt911_config_t gt911_cfg = {
    .i2c_clk_speed = 400*1000,
    .rst_pin = 38, // GPIO 38
#if WITH_GT911_INT
    .int_pin = 18, // GPIO 18
#else
    .int_pin = -1, // INT pin not connected (by default)
#endif
    .swap_xy = false,
    .invert_x = false,
    .invert_y = false,    
};

#ifdef __cplusplus
} /* extern "C" */
#endif
