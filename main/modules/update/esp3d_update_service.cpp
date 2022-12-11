/*
  esp3d_network
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

#include "esp3d_update_service.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "filesystem/esp3d_sd.h"
#include "network/esp3d_network.h"

#define CONFIG_FILE "/esp3dcnf.ini"
#define FW_FILE "/esp3dfw.bin"
#define FW_FILE_OK "/esp3dfw.ok"
#define FS_FILE "/esp3dfs.bin"
#define CHUNK_BUFFER_SIZE 1024

Esp3DUpdateService esp3dUpdateService;

Esp3DUpdateService::Esp3DUpdateService()
{
}

Esp3DUpdateService::~Esp3DUpdateService() {}

bool Esp3DUpdateService::canUpdate()
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *update_partition =  esp_ota_get_next_update_partition(NULL);
    if (!running) {
        esp3d_log_e ("Cannot get running partition");
        return false;
    }
    esp_app_desc_t running_app_info;
    esp3d_log("Running partition subtype %d at offset 0x%lx",
              running->subtype, running->address);
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        esp3d_log("Running firmware version: %s", running_app_info.version);
    }
    if (!update_partition) {
        esp3d_log_e ("Cannot get update partition");
        return false;
    }
    esp3d_log("Update partition subtype %d at offset 0x%lx",
              update_partition->subtype, update_partition->address);
    return true;
}

size_t Esp3DUpdateService::maxUpdateSize()
{
    size_t max_size = 0;
    const esp_partition_t *update_partition =  esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        esp3d_log_e ("Cannot get update partition");
    } else {
        max_size = update_partition->size;
    }
    return max_size;
}

bool Esp3DUpdateService::begin()
{
    esp3d_log("Starting Update Service");
    bool restart =false;
    esp3d_state_t setting_check_update = (esp3d_state_t)esp3dTFTsettings.readByte(esp3d_check_update_on_sd);
    if (setting_check_update==esp3d_state_off || !canUpdate()) {
        esp3d_log("Update Service disabled");
        return true;
    }
    if (sd.accessFS()) {
        if (sd.exists(FW_FILE)) {
            restart= updateFW();
            if (restart) {
                if (!sd.rename(FW_FILE,FW_FILE_OK)) {
                    esp3d_log_e("Failed to rename %s",FW_FILE);
                    //to avoid dead loop
                    restart = false;
                }
            }
        } else {
            esp3d_log("No Fw update on SD");
        }
        sd.releaseFS();
    } else {
        esp3d_log("SD unavailable for update");
    }
    if (restart) {
        esp3d_log("Restarting  firmware");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
        while(1) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    return true;
}

bool Esp3DUpdateService::updateFW()
{
    bool isSuccess = true;
    char chunk[CHUNK_BUFFER_SIZE];
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    esp3d_log("Updating firmware");
    struct stat entry_stat;
    size_t totalSize = 0;
    if (sd.stat(FW_FILE, &entry_stat) == -1) {
        esp3d_log_e("Failed to stat : %s", FW_FILE);
        return false;
    }
    FILE* fwFd = sd.open(FW_FILE, "r");
    if (!fwFd) {
        esp3d_log_e("Failed to open on sd : %s", FW_FILE);
        return false;
    }
    update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        esp3d_log_e("Error accessing flash filesystem");
        isSuccess = false;
    }
    if (isSuccess) {
        esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
        if (err != ESP_OK) {
            esp3d_log_e("esp_ota_begin failed (%s)", esp_err_to_name(err));
            isSuccess = false;
        }
    }
    if (isSuccess) {
        size_t chunksize ;
        uint8_t progress =0;
        do {
            chunksize = fread(chunk, 1, CHUNK_BUFFER_SIZE, fwFd);
            totalSize+=chunksize;
            if (esp_ota_write( update_handle, (const void *)chunk, chunksize)!=ESP_OK) {
                esp3d_log_e("Error cannot writing data on update partition");
                isSuccess = false;
            }
#if ESP3D_TFT_LOG
            if ( progress != 100*totalSize/entry_stat.st_size) {
                progress = 100*totalSize/entry_stat.st_size;
                esp3d_log("Update %d %% %d / %ld", progress, totalSize,entry_stat.st_size );
            }
#endif
        } while (chunksize != 0 && isSuccess);
    }
    sd.close(fwFd);
    if (isSuccess) {
        //check size
        if (totalSize!= entry_stat.st_size) {
            esp3d_log_e("Failed to read firmware full data");
            isSuccess = false;
        }
    }
    if (isSuccess) {
        esp_err_t err = esp_ota_end(update_handle);
        if (err!=ESP_OK) {
            esp3d_log_e("Error cannot end update(%s)", esp_err_to_name(err));
            isSuccess = false;
        }
        update_handle = 0;
    }
    if (isSuccess) {
        esp_err_t err = esp_ota_set_boot_partition(update_partition);
        if (err!=ESP_OK) {
            esp3d_log_e( "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
            isSuccess = false;
        }
    }
    if (update_handle && !isSuccess) {
        esp_ota_abort(update_handle);
        update_handle = 0;
    }
    return isSuccess;
}

void Esp3DUpdateService::handle() {}

void Esp3DUpdateService::end()
{
    esp3d_log("Stop Update Service");
}
