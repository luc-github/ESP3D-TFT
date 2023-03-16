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

#pragma once
#include <esp_mac.h>
#include <stdio.h>

#include <string>

#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "lwip/ip_addr.h"

#ifdef __cplusplus
extern "C" {
#endif
#if ESP3D_WIFI_FEATURE
struct Esp3dIpInfos {
  esp_netif_ip_info_t ip_info;
  esp_netif_dns_info_t dns_info;
};

enum class Esp3dIpMode : uint8_t {
  dhcp = 0,
  staticIp = 1,
};
#endif  // ESP3D_WIFI_FEATURE
enum class Esp3dRadioMode : uint8_t {
  off = 0,
  wifi_sta = 1,
  wifi_ap = 2,
  wifi_ap_config = 3,
  bluetooth_serial = 4
};

class Esp3dNetwork final {
 public:
  Esp3dNetwork();
  ~Esp3dNetwork();
  bool begin();
  void handle();
  void end();
  bool startNoRadioMode();
#if ESP3D_WIFI_FEATURE
  bool startStaMode();
  bool startApMode(bool configMode = false);
  bool startConfigMode();
  bool stopStaMode();
  bool stopApMode();
  bool stopConfigMode();

  const char* getAPMac();
  const char* getSTAMac();

  int32_t getSignal(int32_t RSSI, bool filter = true);
  EventGroupHandle_t getEventGroup() { return _s_wifi_event_group; };
  bool useStaticIp() { return _useStaticIp; };
  bool getLocalIp(Esp3dIpInfos* ipInfo);
  const char* getLocalIpString();

#endif  // ESP3D_WIFI_FEATURE
  bool startBtMode();
  bool stopNoRadioMode();
  bool stopBtMode();
  bool setMode(Esp3dRadioMode mode, bool restart = false);
  const char* getBTMac();
  const char* getMacAddress(uint8_t mac[6]);
  const char* getModeStr(Esp3dRadioMode mode);

  Esp3dRadioMode getMode() { return _current_radio_mode; };
  bool started() { return _started; };

  const char* getHostName() { return _hostname.c_str(); }

 private:
  bool _started;
#if ESP3D_WIFI_FEATURE
  bool _useStaticIp;
  esp_netif_t* _wifiApPtr;
  esp_netif_t* _wifiStaPtr;
  EventGroupHandle_t _s_wifi_event_group;
#endif  // ESP3D_WIFI_FEATURE
  Esp3dRadioMode _current_radio_mode;
  std::string _hostname;
  const char* getMac(esp_mac_type_t type);
};

extern Esp3dNetwork esp3dNetwork;
#ifdef __cplusplus
}  // extern "C"
#endif