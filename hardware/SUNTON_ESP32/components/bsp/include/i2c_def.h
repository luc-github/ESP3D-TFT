// I2C definitions for: ESP32_3248S035C
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_bus.h"

#define I2C_PORT_NUMBER   0

const i2c_config_t i2c_cfg = {
    .mode = I2C_MODE_MASTER,
    .scl_io_num = 32, // GPIO 32
    .sda_io_num = 33, // GPIO 33
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400*1000
};

#ifdef __cplusplus
} /* extern "C" */
#endif
