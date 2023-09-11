// Touch definitions for ESP32_4827S043C
// Touch driver GT911 (I2C)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "GT911"

#include "gt911.h"

gt911_config_t gt911_cfg = {
    .i2c_addr = 0x5D,
    .i2c_clk_speed = 400*1000,
    .rst_pin = 38, // GPIO 38
    .int_pin = -1, // INT pin not connected (by default)
    .swap_xy = false,
    .invert_x = false,
    .invert_y = false,    
};

#ifdef __cplusplus
} /* extern "C" */
#endif
