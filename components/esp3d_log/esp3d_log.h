/*
  esp3d-tft log functions

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
#if ESP3D_TFT_LOG
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
const char *   pathToFileName(const char * path);
#define esp3d_log(format, ...) esp_log_write(ESP_LOG_NONE,"[ESP3D-TFT]","\e[0;36m[%s:%u] %s(): " format "\n" , pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define esp3d_log_e(format, ...) esp_log_write(ESP_LOG_NONE,"[ESP3D-TFT]","\e[0;31m[%s:%u] %s(): " format "\n" , pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define esp3d_log_w(format, ...) esp_log_write(ESP_LOG_NONE,"[ESP3D-TFT]","\e[1;33m[%s:%u] %s(): " format "\n" , pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)

#else
#define esp3d_log(format, ...)
#define esp3d_log_e(format, ...)
#define esp3d_log_w(format, ...)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif