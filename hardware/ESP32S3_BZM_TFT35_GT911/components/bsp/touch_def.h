//Pins definition for ESP32S3_BZM_TFT35_GT911
//Touch driver FT5X06 I2C
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "GT911"

#include "gt911.h"

const gt911_config_t gt911_cfg = {
    .i2c_clk_speed = 400*1000,
    .rst_pin = -1, // GPIO 25
#if WITH_GT911_INT
    .int_pin = 38, // GPIO 38
#else
    .int_pin = -1, // INT pin not connected (by default)
#endif
    .swap_xy = true,
    .invert_x = false,
    .invert_y = true,    
};

#ifdef __cplusplus
} /* extern "C" */
#endif
