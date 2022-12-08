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

    return true;
}

void Esp3DUpdateService::handle() {}

void Esp3DUpdateService::end()
{
    esp3d_log("Stop Update Service");
}
