/*
  esp3d_tft

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

#include "esp3d_tft.h"
#if ESP3D_DISPLAY_FEATURE
#include "esp3d_tft_ui.h"
#endif  // ESP3D_DISPLAY_FEATURE

#include <string>

#include "bsp.h"
#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_tft_network.h"
#include "esp3d_tft_stream.h"
#include "esp_freertos_hooks.h"
#include "filesystem/esp3d_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "version.h"

#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE

#if ESP3D_UPDATE_FEATURE
#include "update/esp3d_update_service.h"
#endif  // ESP3D_UPDATE_FEATURE

/**********************
 *  STATIC PROTOTYPES
 **********************/

Esp3DTFT::Esp3DTFT() {}

Esp3DTFT::~Esp3DTFT() {}

bool Esp3DTFT::begin() {
  // Generic board initialization
  std::string target = TFT_TARGET;
  esp3d_log("Starting ESP3D-TFT on %s ", target.c_str());
  // do nvs init
  esp3d_log("Initialising NVS");
  esp_err_t res = nvs_flash_init();
  if (res == ESP_ERR_NVS_NO_FREE_PAGES ||
      res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    res = nvs_flash_init();
  }
  ESP_ERROR_CHECK(res);
  // Specitic board initialization
  ESP_ERROR_CHECK(bsp_init());

  if (esp3dTFTsettings.isValidSettingsNvs()) {
    esp3d_log("NVS is valid");
    char result[50] = {0};
    if (esp3dTFTsettings.readString(Esp3dSettingIndex::esp3d_version, result,
                                    50)) {
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
#if ESP3D_USB_SERIAL_FEATURE
  if (esp3dCommands.getOutputClient(true) == Esp3dClient::usb_serial) {
    bsp_init_usb();
  } else {
    if (bsp_deinit_usb() != ESP_OK) {
      esp3d_log_e("Failed to deinit usb");
    }
  }
#endif  // #if ESP3D_USB_SERIAL_FEATURE
  bool successFs = flashFs.begin();
  bool successSd = true;
#if ESP3D_SD_CARD_FEATURE
  successSd = sd.begin();
#endif  // ESP3D_SD_CARD_FEATURE
  bool success = true;
#if ESP3D_DISPLAY_FEATURE
  esp3dTFTui.begin();
#endif  // ESP3D_DISPLAY_FEATURE
#if ESP3D_UPDATE_FEATURE
  if (successSd) {
    esp3dUpdateService.begin();
  }
#endif  // ESP3D_UPDATE_FEATURE
  if (success) {
    success = esp3dTFTstream.begin();
  }
  if (success) {
    success = esp3dTFTnetwork.begin();
  }
  return success && successFs && successSd;
}

void Esp3DTFT::handle() {}

bool Esp3DTFT::end() { return true; }
