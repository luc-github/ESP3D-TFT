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
#if ESP3D_WIFI_FEATURE
#include "esp3d_network_services.h"

#include <stdio.h>

#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_values.h"
#include "esp_wifi.h"
#include "http/esp3d_http_service.h"

#if ESP3D_NOTIFICATIONS_FEATURE
#include "notifications/esp3d_notifications_service.h"
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_MDNS_FEATURE
#include "mdns/esp3d_mdns.h"
#endif  // ESP3D_MDNS_FEATURE
#if ESP3D_SSDP_FEATURE
#include "ssdp/esp3d_ssdp.h"
#endif  // ESP3D_SSDP_FEATURE
#if ESP3D_TELNET_FEATURE
#include "socket_server/esp3d_socket_server.h"
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
#include "time/esp3d_time_server.h"
#endif  // ESP3D_TIMESTAMP_FEATURE

#include "authentication/esp3d_authentication.h"

ESP3DNetworkServices esp3dNetworkServices;

ESP3DNetworkServices::ESP3DNetworkServices() { _started = false; }

ESP3DNetworkServices::~ESP3DNetworkServices() {}

bool ESP3DNetworkServices::begin() {
  esp3dTftValues.set_string_value(ESP3DValuesIndex::network_status, "+");
  esp3d_log("Starting Services");
  _started = esp3dAuthenthicationService.begin();
#if ESP3D_TIMESTAMP_FEATURE
  _started = _started && esp3dTimeService.begin();
#endif  // ESP3D_TIMESTAMP_FEATURE

#if ESP3D_HTTP_FEATURE
  _started = _started && esp3dHttpService.begin();
#endif  // ESP3D_HTTP_FEATURE

#if ESP3D_NOTIFICATIONS_FEATURE
  _started = _started && esp3dNotificationsService.begin(true);
#endif  // ESP3D_NOTIFICATIONS_FEATURE

#if ESP3D_MDNS_FEATURE
  _started = _started && esp3dmDNS.begin();
#endif  // ESP3D_MDNS_FEATURE
#if ESP3D_SSDP_FEATURE
  _started = _started && esp3d_ssdp_service.begin();
#endif  // ESP3D_SSDP_FEATURE
#if ESP3D_TELNET_FEATURE
  _started = _started && esp3dSocketServer.begin();
#endif  // ESP3D_TELNET_FEATURE
  return _started;
}

void ESP3DNetworkServices::handle() { esp3dAuthenthicationService.handle(); }

void ESP3DNetworkServices::end() {
  if (!_started) {
    return;
  }
  esp3d_log("Stop Services");
  esp3dAuthenthicationService.end();
#if ESP3D_TELNET_FEATURE
  esp3dSocketServer.end();
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_HTTP_FEATURE
  esp3dHttpService.end();
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_MDNS_FEATURE
  esp3dmDNS.end();
#endif  // ESP3D_MDNS_FEATURE

#if ESP3D_SSDP_FEATURE
  esp3d_ssdp_service.end();
#endif  // ESP3D_SSDP_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
  esp3dNotificationsService.end();
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_TIMESTAMP_FEATURE
  esp3dTimeService.end();
#endif  // ESP3D_TIMESTAMP_FEATURE

  _started = false;
}
#endif  // ESP3D_WIFI_FEATURE