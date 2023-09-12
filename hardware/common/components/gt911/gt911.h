/**
 * @file ft5x06.h
 * @brief FT5x06 driver header file.
 * @version 0.1
 * @date 2021-03-07
 *
 * @copyright Copyright 2021 Espressif Systems (Shanghai) Co. Ltd.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "i2c_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
  uint32_t i2c_clk_speed;
  int8_t rst_pin;
  int8_t int_pin;
  bool swap_xy;
  bool invert_x;
  bool invert_y;
} gt911_config_t;

typedef struct {
  bool is_pressed;
  int16_t x;
  int16_t y;  
} gt911_data_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
extern uint16_t gt911_x_max;
extern uint16_t gt911_y_max;

esp_err_t gt911_init(i2c_bus_handle_t i2c_bus, const gt911_config_t *config);
gt911_data_t gt911_read();

#ifdef __cplusplus
}
#endif
