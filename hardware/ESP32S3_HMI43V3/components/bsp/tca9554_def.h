// Configuration for tca9554 I2C
#pragma once

#include "tca9554.h"

#ifdef __cplusplus
extern "C" {
#endif

const tca9554_config_t tca9554_cfg = {.i2c_clk_speed = 400 * 1000,
                                      .i2c_addr = (uint8_t[]){0x3C, 0x24, 0}};

#ifdef __cplusplus
} /* extern "C" */
#endif
