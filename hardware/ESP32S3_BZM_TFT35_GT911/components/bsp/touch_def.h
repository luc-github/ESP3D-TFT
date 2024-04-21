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
    .i2c_addr =
        (uint8_t[]){
            0x5D, 0X14,
            0},  // Floating or mis-configured INT pin may cause GT911 to come
                 // up at address 0x14 instead of 0x5D, so check there as well.
    .rst_pin = -1, // GPIO25 but already setup by display driver so not needed here
#if WITH_GT911_INT
    .int_pin = 1, // GPIO 1
#else
    .int_pin = -1, // INT pin not connected (by default)
#endif
    .swap_xy = true,
    .invert_x = false,
    .invert_y = true,
    .x_max = 0,//auto detect
    .y_max = 0,//auto detect       
};

#ifdef __cplusplus
} /* extern "C" */
#endif
