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

/*********************
 *      INCLUDES
 *********************/
#include "bsp.h"

#include "esp3d_log.h"
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial.h"
#endif  // ESP3D_USB_SERIAL_FEATURE
#if ESP3D_CAMERA_FEATURE
#include "camera_def.h"
#endif  // ESP3D_CAMERA_FEATURE

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#if ESP3D_USB_SERIAL_FEATURE
/**
 * @brief Initializes the USB functionality of the BSP.
 *
 * This function initializes the USB functionality of the BSP (Board Support
 * Package). It performs any necessary setup and configuration for USB
 * operations.
 *
 * @return esp_err_t Returns `ESP_OK` if the USB initialization is successful,
 * or an error code if it fails.
 */
esp_err_t bsp_init_usb(void) {
  /*usb host initialization */
  esp3d_log("Initializing usb-serial");
  return usb_serial_create_task();
}

/**
 * @brief Deinitializes the USB functionality of the BSP.
 *
 * This function is responsible for deinitializing the USB functionality of the
 * BSP.
 *
 * @return esp_err_t Returns `ESP_OK` on success, or an error code if an error
 * occurred.
 */
esp_err_t bsp_deinit_usb(void) {
  esp3d_log("Remove usb-serial");
  return usb_serial_deinit();
}
#endif  // ESP3D_USB_SERIAL_FEATURE

/**
 * @brief Initializes the Board Support Package (BSP).
 *
 * This function initializes the necessary hardware and peripherals for the
 * board.
 *
 * @return esp_err_t Returns ESP_OK on success, or an error code if
 * initialization fails.
 */
esp_err_t bsp_init(void) {
#if ESP3D_USB_SERIAL_FEATURE
  if (usb_serial_init() != ESP_OK) {
    return ESP_FAIL;
  }
#endif  // ESP3D_USB_SERIAL_FEATURE

#if ESP3D_CAMERA_FEATURE
  if (esp32_camera_init(&camera_config) != ESP_OK) {
    // initialize camera is not critical
    // so failure is not blocking
    esp3d_log_e("Camera init failed");
  }
#endif  // ESP3D_CAMERA_FEATURE
  return ESP_OK;
}
