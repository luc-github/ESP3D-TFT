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

esp_err_t Esp3DHttpService::upload_to_flash_handler(const uint8_t * data, size_t datasize,esp3d_upload_state_t file_upload_state, const char * filename, size_t filesize)
{
    static FILE * FlashFD = nullptr;
    switch(file_upload_state) {
    case upload_file_start:
        esp3d_log("Starting flash upload:%s", filename);
        if (FlashFD) {
            flashFs.close(FlashFD);
            FlashFD = nullptr;
        }
        if (!flashFs.accessFS()) {
            esp3d_log_e("Error accessing flash filesystem");
            esp3dHttpService.pushError(ESP3D_HTTP_ACCESS_ERROR,"Error accessing flash filesystem");
            return ESP_FAIL;
        }
        if (filesize!=(size_t)-1) {
            size_t freespace = 0;
            flashFs.getSpaceInfo(nullptr,nullptr,&freespace);
            if (freespace<filesize) {
                esp3d_log_e("Error not enough space on flash filesystem have %d and need %d",freespace,filesize);
                esp3dHttpService.pushError(ESP3D_HTTP_NOT_ENOUGH_SPACE,"Error not enough space");
                return ESP_FAIL;
            }
        }
        FlashFD = flashFs.open(filename,"w");
        if (!FlashFD) {
            esp3d_log_e("Error cannot create %s on flash filesystem",filename);
            esp3dHttpService.pushError(ESP3D_HTTP_FILE_CREATION,"Error file creation failed");
            return ESP_FAIL;
        }
        break;
    case upload_file_write:
        esp3d_log("Write :%d bytes", datasize);
        if (datasize && FlashFD) {
            if (fwrite(data,datasize,1,FlashFD)!=1) {
                esp3d_log_e("Error cannot writing data on flash filesystem ");
                esp3dHttpService.pushError(ESP3D_HTTP_FILE_WRITE,"Error file write failed");
                return ESP_FAIL;
            }
        }
        break;
    case upload_file_end:
        esp3d_log("Ending upload");
        flashFs.close(FlashFD);
        FlashFD = nullptr;
        if (filesize!=(size_t)-1) {
            struct stat entry_stat;
            if (flashFs.stat(filename, &entry_stat) == -1 || entry_stat.st_size != filesize) {
                if (entry_stat.st_size != datasize) {
                    esp3d_log_e("Invalide size got %d expected %d ",(size_t)entry_stat.st_size, filesize);
                } else {
                    esp3d_log_e("Failed to stat %s",filename);
                }
                flashFs.remove(filename);
                flashFs.releaseFS();
                esp3dHttpService.pushError(ESP3D_HTTP_SIZE,"Error file size does not match expected one");
                return ESP_FAIL;
            }
        }
        flashFs.releaseFS();
        break;
    case upload_file_aborted:
        esp3d_log("Error happened: cleanup");
        flashFs.close(FlashFD);
        FlashFD = nullptr;
        flashFs.remove(filename);
        flashFs.releaseFS();
        break;
    }
    return ESP_OK;
}