// Pins definition for ESP32S3_BZM_TFT35_GT911
// I2C bus
#include "i2c_bus.h"
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define I2C_PORT_NUMBER 0

const i2c_config_t i2c_cfg = {.mode = I2C_MODE_MASTER,
                              .scl_io_num = 2,   // GPIO 2
                              .sda_io_num = 42,  // GPIO 42
                              .scl_pullup_en = GPIO_PULLUP_ENABLE,
                              .sda_pullup_en = GPIO_PULLUP_ENABLE,
                              .master.clk_speed = 400 * 1000};

#ifdef __cplusplus
} /* extern "C" */
#endif
