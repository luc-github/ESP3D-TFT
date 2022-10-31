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

#include "esp3d_network.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "lwip/ip_addr.h"

Esp3DNetwork esp3dNetworkService;

static void wifi_ap_event_handler(void* arg, esp_event_base_t event_base,
                                  int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        esp3d_log("station " MACSTR " join, AID=%d",
                  MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        esp3d_log("station " MACSTR " leave, AID=%d",
                  MAC2STR(event->mac), event->aid);
    }
}

Esp3DNetwork::Esp3DNetwork()
{
    _current_radio_mode = esp3d_radio_off;
}

Esp3DNetwork::~Esp3DNetwork() {}

bool Esp3DNetwork::begin()
{
    static bool bootDone = false;
    esp3d_log("Free mem %d",esp_get_minimum_free_heap_size());
    if (!bootDone) {
        bootDone = true;
        uint8_t bootMode = esp3dTFTsettings.readByte(esp3d_radio_boot_mode);
        if (bootMode==esp3d_radio_off) {
            esp3d_log("Radio is off at boot time");
            _started = true;
            return true;
        }
    }

    if (_current_radio_mode!=esp3d_radio_off) {
        setMode(esp3d_radio_off);
    }

    esp3d_log("Starting Network service...");
    uint8_t radioMode = esp3dTFTsettings.readByte(esp3d_radio_mode);
    _started = setMode((esp3d_radio_mode_t)radioMode);
    return _started;
}

void Esp3DNetwork::handle() {}

void Esp3DNetwork::end()
{
    _started = false;
}
bool Esp3DNetwork::startStaMode()
{
    esp3d_log("Init STA Mode");
    _current_radio_mode = esp3d_wifi_ap;
    return false;
}

bool Esp3DNetwork::startApMode()
{
    static bool initDone   = false;
    char ssid_str[33]= {0};
    char ssid_pwd_str[32]= {0};
    esp3d_log("Init AP Mode");
    esp3d_log("Free mem %d",esp_get_minimum_free_heap_size());
    if (!initDone) {
        esp3d_log("Create default ap service, once");
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        //init the wifi
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        esp3d_log("Free mem %d",esp_get_minimum_free_heap_size());
        if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) {
            esp3d_log_e("Cannot disable persistence of wifi setting to flash");
        }
        //register can be done once also - so no need to unregister if changing to another mode
        esp3d_log("Register wifi handler");
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                        ESP_EVENT_ANY_ID,
                        &wifi_ap_event_handler,
                        NULL,
                        NULL));
        initDone=true;
    }
    esp3d_log("Configure AP settings");
    esp3dTFTsettings.readString(esp3d_ap_ssid,ssid_str, 33);
    uint8_t  channel =  esp3dTFTsettings.readByte(esp3d_ap_channel);
    uint32_t  ip_int =  esp3dTFTsettings.readUint32(esp3d_ap_ip_static);
    esp3dTFTsettings.readString(esp3d_ap_password,ssid_pwd_str, 65);
    esp3d_log("Got  AP settings: SSID:<%s> password:<%s> channel:<%d>",
              ssid_str, ssid_pwd_str, channel);
    wifi_config_t wifi_config ;
    strcpy((char *)wifi_config.ap.ssid,ssid_str);
    strcpy((char *)wifi_config.ap.password,ssid_pwd_str);
    wifi_config.ap.ssid_len = (uint8_t)strlen(ssid_pwd_str);
    wifi_config.ap.channel = channel;
    wifi_config.ap.max_connection = 5;
    if (strlen(ssid_pwd_str) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    } else {
        wifi_config.ap.authmode =  WIFI_AUTH_WPA_WPA2_PSK;
    }
    esp_netif_t* wifiAP = esp_netif_create_default_wifi_ap();
    esp3d_log("Setup IP and DHCP range");
    esp_netif_ip_info_t  ipInfo;
    ipInfo.ip.addr = ip_int;
    ipInfo.gw.addr = ip_int; //0 IF NO GATEWAY
    IP4_ADDR(&ipInfo.netmask, 255,255,255,0);

    esp_netif_dhcps_stop(wifiAP);
    esp_netif_set_ip_info(wifiAP, &ipInfo);
    esp_netif_dhcps_start(wifiAP);

    esp3d_log("set AP mode");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    esp3d_log("Set configuration");
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    esp3d_log("Start wifi");
    ESP_ERROR_CHECK(esp_wifi_start());
#if ESP3D_TFT_LOG
    esp3d_log("wifi_init_softap finished. SSID:%s password:%s channel:%d",
              ssid_str, ssid_pwd_str, channel);
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(wifiAP,&ip_info);
    esp3d_log("AP IP: " IPSTR, IP2STR(&ip_info.ip));
    esp3d_log("AP GW: " IPSTR, IP2STR(&ip_info.gw));
    esp3d_log("AP NETMASK: " IPSTR, IP2STR(&ip_info.netmask));
#endif //ESP3D_TFT_LOG
    _current_radio_mode = esp3d_wifi_ap;
    return true;
}

bool Esp3DNetwork::startConfigMode()
{
    esp3d_log("Init Config Mode");
    bool res = startApMode();
    _current_radio_mode = esp3d_wifi_ap_config;
    return res;
}

bool Esp3DNetwork::startBtMode()
{
    esp3d_log("Init BT Mode");
    _current_radio_mode = esp3d_bluetooth_serial;
    return false;
}

bool  Esp3DNetwork::stopStaMode()
{
    esp3d_log("Stop STA Mode");
    return false;
}
bool  Esp3DNetwork::stopApMode()
{
    esp3d_log("Stop AP Mode");
    return false;
}
bool  Esp3DNetwork::stopConfigMode()
{
    esp3d_log("Stop Config Mode");
    bool res = stopApMode();
    return res;
}
bool  Esp3DNetwork::stopBtMode()
{
    esp3d_log("Stop BT Mode");
    return false;
}

bool Esp3DNetwork::setMode (esp3d_radio_mode_t mode)
{
    switch(_current_radio_mode) {
    case esp3d_radio_off:
        break;
    case esp3d_wifi_sta:
        stopStaMode();
        break;
    case esp3d_wifi_ap:
        stopApMode();
        break;
    case esp3d_wifi_ap_config:
        stopConfigMode();
        break;
    case esp3d_bluetooth_serial:
        stopBtMode();
        break;
    default:
        esp3d_log_e("Unknown radio mode");
    };

    switch(mode) {
    case esp3d_radio_off:
        break;
    case esp3d_wifi_sta:
        startStaMode();
        break;
    case esp3d_wifi_ap:
        startApMode();
        break;
    case esp3d_wifi_ap_config:
        startConfigMode();
        break;
    case esp3d_bluetooth_serial:
        startBtMode();
        break;
    default:
        esp3d_log_e("Unknown radio mode");
    };
    return true;
}