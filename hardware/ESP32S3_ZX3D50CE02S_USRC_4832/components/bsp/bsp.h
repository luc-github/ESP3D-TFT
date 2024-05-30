/*
  esp3d_tft project

  Copyright (c) 2022 Luc Lebosse. All rights reserved.

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
#include "esp_err.h"

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief Initializes the Board Support Package (BSP).
 *
 * This function initializes the necessary hardware and peripherals required by
 * the BSP.
 *
 * @return esp_err_t Returns ESP_OK if the initialization is successful,
 * otherwise an error code.
 */
esp_err_t bsp_init(void);

#if ESP3D_USB_SERIAL_FEATURE
/**
 * @brief Initializes USB in the BSP.
 *
 * This function initializes the USB functionality in the Board Support Package
 * (BSP).
 *
 * @return esp_err_t Returns ESP_OK if the USB initialization is successful,
 * otherwise returns an error code.
 */
esp_err_t bsp_init_usb(void);

/**
 * @brief Deinitializes the USB functionality of the BSP.
 *
 * This function is responsible for deinitializing the USB functionality of the
 * BSP.
 *
 * @return esp_err_t Returns ESP_OK if the USB deinitialization is successful,
 * otherwise returns an error code.
 */
esp_err_t bsp_deinit_usb(void);
#endif  // ESP3D_USB_SERIAL_FEATURE

#ifdef __cplusplus
} /* extern "C" */
#endif
