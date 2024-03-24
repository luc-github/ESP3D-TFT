//I2C definition for ESP32 ESP32S3_ZX3D50CE02S_USRC_4832
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_bus.h"

// I2C definitions
#define I2C_PORT_NUMBER   0

//Configure I2C

//Configure I2C
const i2c_config_t i2c_cfg = {
    .mode = I2C_MODE_MASTER,
    .scl_io_num = 5, // GPIO 5
    .sda_io_num = 6, // GPIO 6
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400*1000
};

#ifdef __cplusplus
} /* extern "C" */
#endif