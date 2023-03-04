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
#include <stdio.h>
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp3d_commands.h"
#include "http/esp3d_http_service.h"
#include "notifications/esp3d_notifications_service.h"
#include "mDNS/esp3d_mdns.h"
#if ESP3D_SSDP_FEATURE
#include "ssdp/esp3d_ssdp.h"
#endif //ESP3D_SSDP_FEATURE
#include "socket_server/esp3d_socket_server.h"
#include "authentication/esp3d_authentication.h"

Esp3DNetworkServices esp3dNetworkServices;

Esp3DNetworkServices::Esp3DNetworkServices()
{
    _started = false;
}

Esp3DNetworkServices::~Esp3DNetworkServices() {}

bool Esp3DNetworkServices::begin()
{
    esp3d_log("Starting Services");
    _started = esp3dAuthenthicationService.begin() && esp3dHttpService.begin() && esp3dNotificationsService.begin(true) && esp3dmDNS.begin();
#if ESP3D_SSDP_FEATURE
    _started = _started && esp3d_ssdp_service.begin();
#endif //ESP3D_SSDP_FEATURE
    _started = _started && esp3dSocketServer.begin();
    return _started;
}

void Esp3DNetworkServices::handle()
{
    esp3dAuthenthicationService.handle();
}

void Esp3DNetworkServices::end()
{
    if (!_started) {
        return;
    }
    esp3d_log("Stop Services");
    esp3dAuthenthicationService.end();
    esp3dSocketServer.end();
    esp3dHttpService.end();
    esp3dmDNS.end();
#if ESP3D_SSDP_FEATURE
    esp3d_ssdp_service.end();
#endif //ESP3D_SSDP_FEATURE
    esp3dNotificationsService.end();
    _started = false;
}
