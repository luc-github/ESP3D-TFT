/*
  esp32_camera.h

  Copyright (c) 2023 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <esp_err.h>
#include <stdbool.h>
#include "esp_camera.h"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
  camera_config_t hw_config;
  int pin_pullup_1;
  int pin_pullup_2;
  int pin_led;
  bool flip_horizontaly;
  bool flip_vertically;
    int brightness;
    int contrast;
} esp32_camera_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
esp_err_t esp32_camera_init(const esp32_camera_config_t *config);

#ifdef __cplusplus
}
#endif
