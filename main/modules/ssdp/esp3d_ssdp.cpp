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
#include "network/esp3d_network.h"
#include "ssdp.h"
#include "ssdp/customizations.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"


Esp3Dssdp esp3d_ssdp_service;

Esp3Dssdp::Esp3Dssdp()
{
    _started=false;
}

Esp3Dssdp::~Esp3Dssdp()
{
    end();
}

const char * Esp3Dssdp::get_schema()
{
    return get_ssdp_schema_str();
}

bool Esp3Dssdp::begin()
{
    if(_started) {
        end();
    }
    ssdp_config_t config = SDDP_DEFAULT_CONFIG();
    config.device_type = (char *)"rootdevice";
    esp_err_t err = ssdp_start(&config);
    if (err!= ESP_OK) {
        esp3d_log_e("Failed to start ssdp: %s", esp_err_to_name(err));
        ssdp_stop();
    } else {
        _started = true;
        esp3d_log("SSDP Service started");
    }

    return _started;
}

void Esp3Dssdp::handle() {}

void Esp3Dssdp::end()
{
    ssdp_stop();
    _started=false;
}