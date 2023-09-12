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

/*********************
 *      DEFINES
 *********************/
#if ESP3D_DISPLAY_FEATURE
#define ESP3D_PATCH_FS_ACCESS_RELEASE 1
#define ESP3D_PATCH_DELAY_REFRESH 1
#endif  // ESP3D_DISPLAY_FEATURE

/**********************
 * GLOBAL PROTOTYPES
 **********************/
esp_err_t bsp_init(void);
esp_err_t bsp_accessFs(void);
esp_err_t bsp_releaseFs(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
