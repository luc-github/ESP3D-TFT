/*
  esp3d-tft

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

#include "esp3d-tft.h"
#include "esp3d-tft-ui.h"
#include "esp3d-tft-stream.h"
#include "esp3d-tft-network.h"
#include "esp3d-settings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "version.h"
#include <string>
#include "esp_log.h"
#include "esp3d_log.h"
#include "bsp.h"
#include "filesystem/esp3d_flash.h"
#include "filesystem/esp3d_sd.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

Esp3DTFT::Esp3DTFT()
{

}

Esp3DTFT::~Esp3DTFT()
{

}

bool Esp3DTFT::begin()
{
    //Generic board initialization
    std::string target =  TFT_TARGET ;
    esp3d_log("Starting ESP3D-TFT on %s ", target.c_str());
    //do nvs init
    esp3d_log("Initialising NVS");
    esp_err_t res = nvs_flash_init();
    if (res == ESP_ERR_NVS_NO_FREE_PAGES || res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        res = nvs_flash_init();
    }
    ESP_ERROR_CHECK(res);
    //Specitic board initialization
    ESP_ERROR_CHECK(bsp_init());


    if (esp3dTFTsettings.isValidSettingsNvs()) {
        esp3d_log("NVS is valid");
        char result[50]= {0};
        if (esp3dTFTsettings.readString(esp3d_version, result,50)) {
            esp3d_log("NVS Setting version is %s", result);
        }
    } else {
        esp3d_log_e("NVS is not valid, need resetting");
        if (esp3dTFTsettings.reset()) {
            esp3d_log("Reset NVS done");
        } else {
            esp3d_log_e("Reset NVS failed");
        }
    }

    bool successFs = flashFs.begin();
    bool successSd = sd.begin();
    bool success =  esp3dTFTui.begin();
    if (success) {
        success = esp3dTFTstream.begin();
    }
    if (success) {
        success = esp3dTFTnetwork.begin();
    }

    return success && successFs && successSd;
}

void Esp3DTFT::handle()
{

}

bool Esp3DTFT::end()
{
    return true;
}
