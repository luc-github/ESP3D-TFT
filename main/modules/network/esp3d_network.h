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
#include <stdio.h>
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    esp3d_ip_mode_dhcp=0,
    esp3d_ip_mode_static=1,
} esp3d_ip_mode_t;

typedef enum {
    esp3d_state_off=0,
    esp3d_state_on=1,
} esp3d_state_t;

typedef enum {
    esp3d_radio_off=0,
    esp3d_wifi_sta=1,
    esp3d_wifi_ap=2,
    esp3d_wifi_ap_config=3,
    esp3d_bluetooth_serial=4
} esp3d_radio_mode_t;


class Esp3DNetwork final
{
public:
    Esp3DNetwork();
    ~Esp3DNetwork();
    bool begin();
    void handle();
    void end();
    bool startNoRadioMode();
    bool startStaMode();
    bool startApMode(bool configMode=false);
    bool startConfigMode();
    bool startBtMode();
    bool stopNoRadioMode();
    bool stopStaMode();
    bool stopApMode();
    bool stopConfigMode();
    bool stopBtMode();
    bool setMode (esp3d_radio_mode_t mode);
    esp3d_radio_mode_t getMode()
    {
        return _current_radio_mode;
    };
    bool started()
    {
        return _started;
    };
private:
    bool _started;
    esp_netif_t* _wifiApPtr;
    esp3d_radio_mode_t _current_radio_mode;
};

extern Esp3DNetwork esp3dNetworkService;
#ifdef __cplusplus
} // extern "C"
#endif