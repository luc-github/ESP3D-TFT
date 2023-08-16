/*
  esp3d_ssdp
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
#include "esp3d_ssdp.h"

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_version.h"
#include "network/esp3d_network.h"
#include "ssdp.h"
#include "ssdp/customizations.h"

ESP3Dssdp esp3d_ssdp_service;

ESP3Dssdp::ESP3Dssdp() { _started = false; }

ESP3Dssdp::~ESP3Dssdp() { end(); }

const char *ESP3Dssdp::get_schema() { return get_ssdp_schema_str(); }

bool ESP3Dssdp::begin() {
  if (_started) {
    end();
  }
  ssdp_init();
  ssdp_config_t config = SDDP_DEFAULT_CONFIG();
  // Customization
  std::string efuseMacStr = std::to_string(esp3d_hal::getEfuseMac());
  std::string friendlyNameStr = esp3dNetwork.getHostName();
  // Friendly name
  config.friendly_name = friendlyNameStr.c_str();
  // Serial Number
  config.serial_number = efuseMacStr.c_str();
  // Http port
  uint32_t portValue =
      esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_http_port);
  config.port = portValue;
  // User customization if any
  // Modele name
#if defined(ESP3D_MODEL_NAME)
  config.model_name = ESP3D_MODEL_NAME;
#else
  config.model_name = TFT_TARGET;
#endif
  // Modele Number
#if defined(ESP3D_MODEL_NUMBER)
  config.model_number = ESP3D_MODEL_NUMBER;
#else
  config.model_number = ESP3D_TFT_VERSION;
#endif
  // Modele Url
#if defined(ESP3D_MODEL_URL)
  config.model_url = ESP3D_MODEL_URL;
#else
  config.model_url = ESP3D_TFT_FW_URL;
#endif
  // Modele description
  // this one is optional because windows doesn't care about this field
#if defined(ESP3D_MODEL_DESCRIPTION)
  config.model_description = ESP3D_MODEL_URL;
#endif
  // Manufacturer Name
#if defined(ESP3D_MANUFACTURER_NAME)
  config.manufacturer_name = ESP3D_MANUFACTURER_NAME;
#endif
  // Manufacturer URL
#if defined(ESP3D_MANUFACTURER_URL)
  config.manufacturer_url = ESP3D_MANUFACTURER_URL;
#endif
  // Note: other fiels are left as default values
  // because no need to let user to customize them here

  config.device_type = (char *)"rootdevice";
  esp_err_t err = ssdp_start(&config);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to start ssdp: %s", esp_err_to_name(err));
    ssdp_stop();
  } else {
    _started = true;
    esp3d_log("SSDP Service started");
  }

  return _started;
}

void ESP3Dssdp::handle() {}

void ESP3Dssdp::end() {
  esp3d_log("Stop SSDP service");
  ssdp_stop();
  _started = false;
}