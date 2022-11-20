/*
  esp3d_http_service : upload_to_flash_handler
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


#include "http/esp3d_http_service.h"
#include <sys/param.h>
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp3d_commands.h"
#include "filesystem/esp3d_flash.h"

esp_err_t Esp3DHttpService::upload_to_flash_handler(const uint8_t * data, size_t datasize,esp3d_upload_state_t file_upload_state,  FILE * fd, const char * filename, size_t filesize)
{
    switch(file_upload_state) {
    case upload_file_start:
        esp3d_log("Starting flash upload:%s", filename);
        break;
    case upload_file_write:
        esp3d_log("Write :%d bytes", datasize);
        break;
    case upload_file_end:
        esp3d_log("Ending upload");
        break;
    case upload_file_aborted:
        esp3d_log("Error happened: cleanup");
        break;
    }
    return ESP_OK;
}