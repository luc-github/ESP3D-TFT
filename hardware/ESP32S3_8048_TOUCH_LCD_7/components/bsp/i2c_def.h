// I2C definitions for ESP32S3_8048_TOUCH_LCD_7
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_bus.h"

#define I2C_PORT_NUMBER 0

const i2c_config_t i2c_cfg = {.mode = I2C_MODE_MASTER,
                              .scl_io_num = 9,  // GPIO 9
                              .sda_io_num = 8,  // GPIO 8
                              .scl_pullup_en = GPIO_PULLUP_ENABLE,
                              .sda_pullup_en = GPIO_PULLUP_ENABLE,
                              .master.clk_speed = 400 * 1000};

#ifdef __cplusplus
} /* extern "C" */
#endif
