/*
  esp3d_http_service : upload_to_updatefw_handler
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
#if ESP3D_UPDATE_FEATURE

#include "http/esp3d_http_service.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp_ota_ops.h"

esp_err_t Esp3DHttpService::upload_to_updatefw_handler(const uint8_t * data, size_t datasize,esp3d_upload_state_t file_upload_state, const char * filename, size_t filesize)
{
    //No need Authentication as already handled in multipart_parser
    static esp_ota_handle_t update_handle = 0;
    static const esp_partition_t *update_partition = NULL;
    static size_t update_size = 0;
    esp_err_t err;
    switch(file_upload_state) {
    case upload_file_start:
        esp3d_log("Starting update upload");
        update_size = 0;
        update_partition = esp_ota_get_next_update_partition(NULL);
        if (!update_partition) {
            esp3d_log_e("Error accessing flash filesystem");
            esp3dHttpService.pushError(ESP3D_HTTP_START_UPDATE,"Error accessing update partition");
            return ESP_FAIL;
        }
        if (filesize!=(size_t)-1) {
            if (update_partition->size<filesize) {
                esp3d_log_e("Error not enough space on flash filesystem have %ld and need %d",update_partition->size,filesize);
                esp3dHttpService.pushError(ESP3D_HTTP_NOT_ENOUGH_SPACE,"Error not enough space");
                return ESP_FAIL;
            }
        }
        err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
        if (err != ESP_OK) {
            esp3d_log_e("esp_ota_begin failed (%s)", esp_err_to_name(err));
            esp3dHttpService.pushError(ESP3D_HTTP_FILE_CREATION,"Error update begin failed");
            return ESP_FAIL;
        }
        break;
    case upload_file_write:
        //esp3d_log("Write :%d bytes", datasize);
        if (datasize && update_handle) {
            if (esp_ota_write( update_handle, (const void *)data, datasize)!=ESP_OK) {
                esp3d_log_e("Error cannot writing data on update partition ");
                esp3dHttpService.pushError(ESP3D_HTTP_FILE_WRITE,"Error update write failed");
                return ESP_FAIL;
            }
            update_size+=datasize;
        }
        break;
    case upload_file_end:
        esp3d_log("Ending upload");
        if (filesize!=(size_t)-1) {
            if (filesize !=update_size) {
                esp3d_log_e("Invalid size got %d expected %d ",update_size, filesize);
                std::string tmperr = "Error file size does not match, got";
                tmperr +=std::to_string(update_size);
                tmperr +=" vs ";
                tmperr +=std::to_string(filesize);
                esp3dHttpService.pushError(ESP3D_HTTP_SIZE,tmperr.c_str());
                return ESP_FAIL;
            } else {
                esp3d_log("Final size is Ok");
            }
        }
        err = esp_ota_end(update_handle);
        if (err!=ESP_OK) {
            esp3d_log_e("Error cannot end update(%s)", esp_err_to_name(err));
            std::string tmperr = "Error update failed(";
            tmperr += esp_err_to_name(err);
            tmperr +=")";
            esp3dHttpService.pushError(ESP3D_HTTP_FILE_CLOSE,tmperr.c_str());
            return ESP_FAIL;
        }
        err = esp_ota_set_boot_partition(update_partition);
        if (err!=ESP_OK) {
            esp3d_log_e( "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
            std::string tmperr = "Error update failed(";
            tmperr += esp_err_to_name(err);
            tmperr +=")";
            esp3dHttpService.pushError(ESP3D_HTTP_UPDATE,tmperr.c_str());
            return ESP_FAIL;
        }
        break;
    case upload_file_aborted:
        esp3d_log("Error happened: cleanup");
        esp_ota_abort(update_handle);
        update_handle = 0;
        break;
    }
    return ESP_OK;
}
#endif //ESP3D_UPDATE_FEATURE