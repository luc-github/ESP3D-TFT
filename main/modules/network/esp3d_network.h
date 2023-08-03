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
struct ESP3DIpInfos {
  esp_netif_ip_info_t ip_info;
  esp_netif_dns_info_t dns_info;
};

enum class ESP3DIpMode : uint8_t {
  dhcp = 0,
  staticIp = 1,
};
#endif  // ESP3D_WIFI_FEATURE
enum class ESP3DRadioMode : uint8_t {
  off = 0,
  wifi_sta = 1,
  wifi_ap = 2,
  wifi_ap_config = 3,
  bluetooth_serial = 4,
  wifi_ap_limited = 6,
  none = 7,
};

class ESP3DNetwork final {
 public:
  ESP3DNetwork();
  ~ESP3DNetwork();
  bool begin();
  void handle();
  void end();
  bool startNoRadioMode();
#if ESP3D_WIFI_FEATURE
  bool startStaMode();
  bool startApMode(bool configMode = false, bool limited = false);
  bool startConfigMode();
  bool startLimitedMode();
  bool stopStaMode();
  bool stopApMode();
  bool stopConfigMode();
  bool stopLimitedMode();

  const char* getAPMac();
  const char* getSTAMac();

  int32_t getSignal(int32_t RSSI, bool filter = true);
  EventGroupHandle_t getEventGroup() { return _s_wifi_event_group; };
  bool useStaticIp() { return _useStaticIp; };
  bool getLocalIp(ESP3DIpInfos* ipInfo);
  const char* getLocalIpString();

#endif  // ESP3D_WIFI_FEATURE
  bool startBtMode();
  bool stopNoRadioMode();
  bool stopBtMode();
  bool setMode(ESP3DRadioMode mode, bool restart = false);
  bool setModeAsync(ESP3DRadioMode mode);
  const char* getBTMac();
  const char* getMacAddress(uint8_t mac[6]);
  const char* getModeStr(ESP3DRadioMode mode);

  ESP3DRadioMode getMode() { return _current_radio_mode; };
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
  ESP3DRadioMode _current_radio_mode;
  ESP3DRadioMode _async_radio_mode;
  std::string _hostname;
  const char* getMac(esp_mac_type_t type);
};

extern ESP3DNetwork esp3dNetwork;
#ifdef __cplusplus
}  // extern "C"
#endif