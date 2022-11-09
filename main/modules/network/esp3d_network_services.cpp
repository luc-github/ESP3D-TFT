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

#include "esp3d_network_services.h"
#include "esp3d_network.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp3d_commands.h"

Esp3DNetworkServices esp3dNetworkServices;

Esp3DNetworkServices::Esp3DNetworkServices()
{
    _started = false;
}

Esp3DNetworkServices::~Esp3DNetworkServices() {}

bool Esp3DNetworkServices::begin()
{
    esp3d_log("Starting Services");
    _started = true;
    return _started;
}

void Esp3DNetworkServices::handle() {}

void Esp3DNetworkServices::end()
{
    if (!_started) {
        return;
    }
    esp3d_log("Stop Services");
    _started = false;
}
