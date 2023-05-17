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
#include "esp3d_log.h"
#include "bsp.h"
#include "usb_serial.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
esp_err_t bsp_init_usb(void)
{
    /*usb host initialization */
    esp3d_log("Initializing usb-serial");
    return usb_serial_create_task();
}

esp_err_t bsp_deinit_usb(void)
{
    esp3d_log("Remove usb-serial");
    return usb_serial_deinit();
}

esp_err_t bsp_init(void)
{
    if ( usb_serial_init()!=ESP_OK) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

