//Pins definition for ESP32S3_ZX3D50CE02S_USRC_4832
//Touch driver FT5X06 I2C
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_CONTROLLER "FT5X06"

#include "ft5x06.h"

const ft5x06_config_t ft5x06_cfg = {
    .i2c_clk_speed = 400*1000,
    .i2c_addr =
        (uint8_t[]){
            0x38,
            0}, 
    .rst_pin = -1,//GPIO 4 same as panel so no need to define it again
#if WITH_FT5X06_INT
    .int_pin = 7, // GPIO 7
#else
    .int_pin = -1, // INT pin not connected (by default)
#endif
    .swap_xy = true,
    .invert_x = false,
    .invert_y = false,
    .x_max = 320,
    .y_max = 480,     
};                      

#ifdef __cplusplus
} /* extern "C" */
#endif
