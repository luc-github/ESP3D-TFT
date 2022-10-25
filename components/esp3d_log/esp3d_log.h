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
#if DISABLE_COLOR_LOG
#define LOG_COLOR_NORMAL ""
#define LOG_COLOR_ERROR ""
#define LOG_COLOR_WARNING ""
#define LOG_NO_COLOR ""
#else
#define LOG_COLOR_NORMAL "\e[0;36m"
#define LOG_COLOR_ERROR "\e[0;31m"
#define LOG_COLOR_WARNING "\e[1;33m"
#define LOG_NO_COLOR "\e[0;37m\e[0m"
#endif

const char *   pathToFileName(const char * path);
#define esp3d_log(format, ...) esp_log_write(ESP_LOG_NONE,"[ESP3D-TFT]","%s[%s:%u] %s(): " format "%s\n" , LOG_COLOR_NORMAL,pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__,LOG_NO_COLOR)
#define esp3d_log_e(format, ...) esp_log_write(ESP_LOG_NONE,"[ESP3D-TFT]","%s[%s:%u] %s(): " format "%s\n" , LOG_COLOR_ERROR, pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__,LOG_NO_COLOR)
#define esp3d_log_w(format, ...) esp_log_write(ESP_LOG_NONE,"[ESP3D-TFT]","%s[%s:%u] %s(): " format "%s\n" , LOG_COLOR_WARNING, pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__,LOG_NO_COLOR)

#else
#define esp3d_log(format, ...)
#define esp3d_log_e(format, ...)
#define esp3d_log_w(format, ...)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif