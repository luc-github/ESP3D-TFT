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
#include "esp3d_commands.h"
#include "lwip/apps/netbiosns.h"
#include "esp3d_network_services.h"

Esp3DNetwork esp3dNetwork;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_STA_LOST_IP   BIT2

#define MIN_RSSI           -78
#define ESP3D_STA_MAXIMUM_RETRY 10

static void wifi_ap_event_handler(void* arg, esp_event_base_t event_base,
                                  int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
#if ESP3D_TFT_LOG
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        esp3d_log("station " MACSTR " join, AID=%d",
                  MAC2STR(event->mac), event->aid);
#else
        //TODO: TBD
#endif // ESP3D_TFT_LOG
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
#if ESP3D_TFT_LOG
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        esp3d_log("station " MACSTR " leave, AID=%d",
                  MAC2STR(event->mac), event->aid);
#else
        //TODO: TBD
#endif // ESP3D_TFT_LOG
    }
}

static void  wifi_sta_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    static uint s_retry_num = 0;
    //IP_EVENT_STA_LOST_IP TODO:

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp3d_log( "Try to connect to the AP");
        esp_err_t  res = esp_wifi_connect();
        if (res == ESP_OK) {
            esp3d_log( "Success connecting to the AP");
            if (esp3dNetwork.useStaticIp()) {
                xEventGroupSetBits(esp3dNetwork.getEventGroup(), WIFI_CONNECTED_BIT);
                s_retry_num = 0;
            }
        } else {
            esp3d_log_e("connect to the AP failed: %s", esp_err_to_name(res));
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP3D_STA_MAXIMUM_RETRY) {
            esp3d_log( "retry to connect to the AP %d", s_retry_num);
            esp_err_t  res = esp_wifi_connect();
            s_retry_num++;
            if (res == ESP_OK) {
                esp3d_log( "Success connecting to the AP");
                if (esp3dNetwork.useStaticIp()) {
                    s_retry_num = 0;
                    xEventGroupSetBits(esp3dNetwork.getEventGroup(), WIFI_CONNECTED_BIT);
                }
            } else {
                esp3d_log_e("connect to the AP failed: %s", esp_err_to_name(res));
            }
        } else {
            esp3d_log_e("retries to connect to the AP failed");
            xEventGroupSetBits(esp3dNetwork.getEventGroup(), WIFI_FAIL_BIT);
        }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
#if ESP3D_TFT_LOG
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        esp3d_log("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
#else
        //TODO: TBD
#endif // ESP3D_TFT_LOG
        s_retry_num = 0;
        xEventGroupSetBits(esp3dNetwork.getEventGroup(), WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
        esp3d_log("Client IP was lost");
        xEventGroupSetBits(esp3dNetwork.getEventGroup(), WIFI_STA_LOST_IP);
    }
}

bool  Esp3DNetwork::getLocalIp(esp3d_ip_info_t * ipInfo)
{
    if (!ipInfo) {
        return false;
    }
    ipInfo->ip_info.ip.addr = 0;
    ipInfo->ip_info.gw.addr = 0;
    ipInfo->ip_info.netmask.addr = 0;
    ipInfo->dns_info.ip.u_addr.ip4.addr = 0;

    switch (_current_radio_mode) {
    case esp3d_wifi_sta:
        if (_wifiStaPtr) {
            if (ESP_OK == esp_netif_get_ip_info(_wifiStaPtr,&(ipInfo->ip_info))) {
                if (ESP_OK == esp_netif_get_dns_info(_wifiStaPtr, ESP_NETIF_DNS_MAIN, &(ipInfo->dns_info))) {
                    return true;
                }
            }
        }
        break;
    case esp3d_wifi_ap_config:
    case esp3d_wifi_ap:
        if (_wifiApPtr) {
            if (ESP_OK == esp_netif_get_ip_info(_wifiApPtr,&(ipInfo->ip_info))) {
                if (ESP_OK == esp_netif_get_dns_info(_wifiApPtr, ESP_NETIF_DNS_MAIN, &(ipInfo->dns_info))) {
                    return true;
                }
            }
        }
        break;
    case esp3d_bluetooth_serial:
    case esp3d_radio_off:
        return true;
    default:
        break;
    }

    return false;
}

Esp3DNetwork::Esp3DNetwork()
{
    _current_radio_mode = esp3d_radio_off;
    _wifiApPtr = nullptr;
    _s_wifi_event_group = nullptr;
    _useStaticIp = false;
    _started = false;
}

Esp3DNetwork::~Esp3DNetwork() {}

bool Esp3DNetwork::begin()
{
    static bool bootDone = false;
    esp3d_log("Free mem %ld",esp_get_minimum_free_heap_size());
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

void Esp3DNetwork::handle()
{
    if (_current_radio_mode==esp3d_wifi_sta && ( xEventGroupGetBits(_s_wifi_event_group)& WIFI_STA_LOST_IP)) {
        xEventGroupClearBits(_s_wifi_event_group, WIFI_STA_LOST_IP);
        esp3d_log("Force restart wifi station");
        setMode (esp3d_wifi_sta, true);
    }
}

void Esp3DNetwork::end()
{
    _started = false;
    _useStaticIp= false;
}

const char * Esp3DNetwork::getAPMac()
{
    return getMac(ESP_MAC_WIFI_SOFTAP);
}
const char * Esp3DNetwork::getSTAMac()
{
    return getMac(ESP_MAC_WIFI_STA);
}
const char * Esp3DNetwork::getBTMac()
{
    return getMac(ESP_MAC_BT);
}

const char * Esp3DNetwork::getMacAddress(uint8_t mac[6])
{
    static char mac_addr[18]= { 0 };
    sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return mac_addr;
}

const char * Esp3DNetwork::getMac(esp_mac_type_t type)
{
    uint8_t mac[6];
    if (ESP_OK != esp_read_mac(mac, type)) {
        memset(mac, 0, 6);
    }
    return getMacAddress(mac);
}

const char * Esp3DNetwork::getModeStr(esp3d_radio_mode_t mode)
{
    switch(mode) {
    case esp3d_radio_off:
        return "No Radio";
    case esp3d_wifi_sta:
        return "Client";
    case esp3d_wifi_ap:
        return "Access point";
    case esp3d_wifi_ap_config:
        return "Configuration";
    case esp3d_bluetooth_serial:
        return "Bluetooth";
    default:
        break;
    }
    return "Unknown";
}

bool Esp3DNetwork::startStaMode()
{
    bool connected = false;
    char ssid_str[33]= {0};
    char ssid_pwd_str[32]= {0};
    if (_s_wifi_event_group) {
        stopStaMode();
    }
    esp3d_log("Init STA Mode");
    esp3dTFTsettings.readString(esp3d_sta_ssid,ssid_str, 33);
    if (strlen(ssid_str)==0) {
        return false;
    }
    _s_wifi_event_group = xEventGroupCreate();
    if (!_s_wifi_event_group) {
        esp3d_log("Cannot create event group");
        return false;
    }
    _wifiStaPtr = esp_netif_create_default_wifi_sta();
    if (!_wifiStaPtr) {
        esp3d_log_e("Cannot create default wifi station");
        return false;
    }
    esp3d_log("Free mem %ld",esp_get_minimum_free_heap_size());
    esp3d_log("Create default sta service");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    //init the wifi
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp3d_log("Free mem %ld",esp_get_minimum_free_heap_size());
    if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) {
        esp3d_log_e("Cannot disable persistence of wifi setting to flash");
    }

    esp3d_log("Register wifi handler");

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_sta_event_handler,
                    NULL,
                    NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_sta_event_handler,
                    NULL,
                    NULL));
    esp3d_log("Configure STA settings");
    esp3dTFTsettings.readString(esp3d_sta_ssid,ssid_str, 33);
    esp3dTFTsettings.readString(esp3d_sta_password,ssid_pwd_str, 65);
    uint8_t  ipMode =  esp3dTFTsettings.readByte(esp3d_sta_ip_mode);
    esp3d_log("Got  STA settings: SSID:<%s> password:<%s> ip mode :<%s>",
              ssid_str, ssid_pwd_str, ipMode== esp3d_ip_mode_static?"Static":"DHCP");
    wifi_config_t wifi_config ;
    strcpy((char *)wifi_config.sta.ssid,ssid_str);
    strcpy((char *)wifi_config.sta.password,ssid_pwd_str);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    wifi_config.sta.scan_method = WIFI_FAST_SCAN;
    wifi_config.sta.bssid_set = 0;
    memset(wifi_config.sta.bssid, 0, 6);
    wifi_config.sta.channel = 0;
    wifi_config.sta.threshold.rssi = -127;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.listen_interval = 3;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    if ((esp3d_ip_mode_t)ipMode == esp3d_ip_mode_static ) {
        _useStaticIp = true;
        esp3d_log("Set IP static mode");
        esp_netif_dhcpc_stop(_wifiStaPtr);
        uint32_t  ip_int =  esp3dTFTsettings.readUint32(esp3d_sta_ip_static);
        uint32_t  gw_int =  esp3dTFTsettings.readUint32(esp3d_sta_gw_static);
        uint32_t  msk_int =  esp3dTFTsettings.readUint32(esp3d_sta_mask_static);
        uint32_t  dns_int =  esp3dTFTsettings.readUint32(esp3d_sta_dns_static);
        esp_netif_ip_info_t ip_info;
        ip_info.ip.addr = ip_int;
        ip_info.gw.addr = gw_int;
        ip_info.netmask.addr = msk_int;
        esp_netif_set_ip_info(_wifiStaPtr, &ip_info);
        esp_netif_dns_info_t	dns_info;
        dns_info.ip.u_addr.ip4.addr= dns_int;
        esp_netif_set_dns_info(_wifiStaPtr, ESP_NETIF_DNS_MAIN, &dns_info);
    } else {
        esp3d_log("Use DHCP");
        _useStaticIp = false;
        esp_err_t  res = esp_netif_dhcpc_start(_wifiStaPtr);
        if (ESP_OK == res) {
            esp3d_log("Dhcp client started");
        } else if(res ==  ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED) {
            esp3d_log("Dhcp  client already started");
        } else {
            esp3d_log_e("Cannot get start dhcp client");
        }

    }
    std::string stmp;
    esp3d_request_t requestId= {.id=0};
    stmp = "Connection to ";
    stmp += ssid_str;
    stmp +="\n";
    esp3dCommands. dispatch(stmp.c_str(),  ALL_CLIENTS,requestId, msg_unique,ESP3D_SYSTEM, ESP3D_LEVEL_ADMIN);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );


    //Set hostname
    const esp3d_setting_desc_t * settingPtr = esp3dTFTsettings.getSettingPtr(esp3d_hostname);
    std::string hostname;
    if (settingPtr) {
        char out_str[33]= {0};
        hostname = esp3dTFTsettings.readString(esp3d_hostname, out_str, settingPtr->size);
        if (ESP_OK != esp_netif_set_hostname(_wifiStaPtr, hostname.c_str())) {
            esp3d_log_e("Failed to set hostname");
        }
    } else {
        esp3d_log_e("No hostname setting found");
    }
    netbiosns_init();
    netbiosns_set_name(hostname.c_str());
    ESP_ERROR_CHECK(esp_wifi_start() );
    _current_radio_mode = esp3d_wifi_sta;
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(_s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    // xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
    //happened.
    if (bits & WIFI_CONNECTED_BIT) {
        esp3d_log("connected to ap SSID: %s, password: %s",
                  ssid_str, ssid_pwd_str);
        esp_netif_ip_info_t ip_info;
        if (ESP_OK == esp_netif_get_ip_info(_wifiStaPtr,&ip_info)) {
            stmp = ip4addr_ntoa((const ip4_addr_t*)&ip_info.ip);
            stmp+="\n";
            connected = true;
        }
    } else if (bits & WIFI_FAIL_BIT) {
        esp3d_log_e("Failed to connect to SSID: %s, password: %s",
                    ssid_str, ssid_pwd_str);
    } else {
        esp3d_log_e("UNEXPECTED EVENT");
    }
    xEventGroupClearBits(_s_wifi_event_group, WIFI_CONNECTED_BIT);
    xEventGroupClearBits(_s_wifi_event_group, WIFI_FAIL_BIT);
    if (!connected) {
        stmp = "Connect to ";
        stmp +=ssid_str;
        stmp += " failed\n";
    }
    esp3dCommands. dispatch(stmp.c_str(),  ALL_CLIENTS,requestId, msg_unique, ESP3D_SYSTEM, ESP3D_LEVEL_ADMIN);
    if (connected) {
        esp3dNetworkServices.begin();
    }
    return connected;
}

int32_t Esp3DNetwork::getSignal (int32_t RSSI, bool filter)
{
    if (RSSI < MIN_RSSI && filter) {
        return 0;
    }
    if (RSSI <= -100 && !filter) {
        return 0;
    }
    if (RSSI >= -50) {
        return 100;
    }
    return (2 * (RSSI + 100) );

}

bool Esp3DNetwork::startApMode(bool configMode)
{
    bool success = false;
    char ssid_str[33]= {0};
    char ssid_pwd_str[32]= {0};
    if (_wifiApPtr) {
        stopApMode();
    }
    esp3d_log("Init AP Mode");
    esp3d_log("Free mem %ld",esp_get_minimum_free_heap_size());

    esp3d_log("Create default ap service");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    //init the wifi
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp3d_log("Free mem %ld",esp_get_minimum_free_heap_size());
    if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK) {
        esp3d_log_e("Cannot disable persistence of wifi setting to flash");
    }

    esp3d_log("Configure AP settings");
    //register can be done once also - so no need to unregister if changing to another mode
    esp3d_log("Register wifi handler");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_ap_event_handler,
                    NULL,
                    NULL));
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
    _wifiApPtr = esp_netif_create_default_wifi_ap();
    if (!_wifiApPtr) {
        esp3d_log_e("Create default wifi AP failed");
        return false;
    }
    esp3d_log("Setup IP and DHCP range");
    esp_netif_ip_info_t  ipInfo;
    ipInfo.ip.addr = ip_int;
    ipInfo.gw.addr = configMode?ip_int:0; //0 = no gateway address
    IP4_ADDR(&ipInfo.netmask, 255,255,255,0);

    esp_netif_dhcps_stop(_wifiApPtr);
    esp_netif_set_ip_info(_wifiApPtr, &ipInfo);
    esp_netif_dhcps_start(_wifiApPtr);

    esp3d_log("set AP mode");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    esp3d_log("Set configuration");
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    esp3d_log("Start wifi");
    ESP_ERROR_CHECK(esp_wifi_start());
    //Set hostname
    const esp3d_setting_desc_t * settingPtr = esp3dTFTsettings.getSettingPtr(esp3d_hostname);
    if (settingPtr) {
        char out_str[33]= {0};
        std::string hostname = esp3dTFTsettings.readString(esp3d_hostname,out_str, settingPtr->size);
        if (ESP_OK != esp_netif_set_hostname(_wifiApPtr,hostname.c_str())) {
            esp3d_log_e("Failed to set hostname");
        }
    } else {
        esp3d_log_e("No hostname setting found");
    }


    esp_netif_ip_info_t ip_info;
    std::string stmp;
    esp3d_request_t requestId= {.id=0};
    _current_radio_mode = esp3d_wifi_ap;
    if (ESP_OK == esp_netif_get_ip_info(_wifiApPtr,&ip_info)) {
#if ESP3D_TFT_LOG
        esp3d_log("wifi_init_softap done.\nSSID:%s\npassword:%s\nchannel:%d",
                  ssid_str, ssid_pwd_str, channel);
        esp3d_log("AP IP: " IPSTR, IP2STR(&ip_info.ip));
        esp3d_log("AP GW: " IPSTR, IP2STR(&ip_info.gw));
        esp3d_log("AP NETMASK: " IPSTR, IP2STR(&ip_info.netmask));
#endif //ESP3D_TFT_LOG
        stmp = "Access Point ";
        if(configMode) {
            stmp += "(config mode) ";
        }
        stmp += ssid_str;
        stmp += " started";
        if (strlen(ssid_pwd_str) > 0) {
            stmp+=" protected with ";
        } else {
            stmp+=" without ";
        }
        stmp+= "password (";
        stmp+= ip4addr_ntoa((const ip4_addr_t*)&ip_info.ip);
        stmp+= ")\n";
        success= true;
    } else {
        stmp = "Access Point setup failed\n";
    }
#if ESP3D_TFT_LOG
    //to avoid esp3d_log merge with dispatch message on serial output
    vTaskDelay(pdMS_TO_TICKS(500));
#endif // ESP3D_TFT_LOG
    esp3dCommands. dispatch(stmp.c_str(),  ALL_CLIENTS,requestId, msg_unique, ESP3D_SYSTEM, ESP3D_LEVEL_ADMIN);
    if (success) {
        esp3dNetworkServices.begin();
    }
    return success;
}

bool  Esp3DNetwork::startNoRadioMode()
{
    esp3d_log("Start No Radio Mode");
    std::string stmp = "Radio is off\n";
    _current_radio_mode = esp3d_radio_off;
    esp3d_request_t requestId= {.id=0};
    esp3dCommands. dispatch(stmp.c_str(),  ALL_CLIENTS,requestId, msg_unique, ESP3D_SYSTEM, ESP3D_LEVEL_ADMIN);
    return true;
}

bool  Esp3DNetwork::stopNoRadioMode()
{
    esp3d_log("Stop No Radio Mode");
    return true;
}

bool Esp3DNetwork::startConfigMode()
{
    esp3d_log("Init Config Mode");
    bool res = startApMode(true);
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
    if (!(_current_radio_mode==esp3d_wifi_sta)) {
        return false;
    }
    esp3d_log("Stop STA Mode");
    esp3dNetworkServices.end();
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_sta_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_sta_event_handler));
    if (_s_wifi_event_group) {
        esp3d_log("Clear xEventGroupBits");
        xEventGroupClearBits(_s_wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupClearBits(_s_wifi_event_group, WIFI_FAIL_BIT);
        xEventGroupClearBits(_s_wifi_event_group, WIFI_STA_LOST_IP);
        vEventGroupDelete(_s_wifi_event_group);
        _s_wifi_event_group = nullptr;
    }
    if (ESP_OK!=esp_wifi_disconnect()) {
        esp3d_log_w("Disconnection failed");
    }

    if(esp_wifi_stop()!=ESP_OK) {
        esp3d_log_e("Cannot stop wifi");
        return false;
    }
    if (_wifiStaPtr) {
        esp3d_log("Stop DHCP Client");
        esp_netif_dhcpc_stop(_wifiStaPtr);
        ESP_ERROR_CHECK(esp_wifi_deinit());
        ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(_wifiStaPtr));
        esp_netif_destroy_default_wifi(_wifiStaPtr);
        _wifiStaPtr = nullptr;
    } else {
        esp3d_log_w("_wifiStaPtr is null");
    }
    _current_radio_mode = esp3d_radio_off;
    return true;
}

bool  Esp3DNetwork::stopApMode()
{
    if (!(_current_radio_mode==esp3d_wifi_ap_config || _current_radio_mode==esp3d_wifi_ap)) {
        return false;
    }
    esp3d_log("Stop AP Mode");

    esp3dNetworkServices.end();
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,
                    ESP_EVENT_ANY_ID,
                    &wifi_ap_event_handler));
    if(esp_wifi_stop()!=ESP_OK) {
        esp3d_log_e("Cannot stop wifi");
        return false;
    }
    if (_wifiApPtr) {
        esp3d_log("Stop DHCP");
        esp_netif_dhcps_stop(_wifiApPtr);
        esp3d_log("Destroy default wifi AP");
        ESP_ERROR_CHECK(esp_wifi_deinit());
        ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(_wifiApPtr));
        esp_netif_destroy_default_wifi(_wifiApPtr);
        _wifiApPtr = nullptr;
    } else {
        esp3d_log_w("_wifiApPtr is null");
    }

    _current_radio_mode = esp3d_radio_off;
    return true;
}
bool  Esp3DNetwork::stopConfigMode()
{
    esp3d_log("Stop Config Mode");

    esp3dNetworkServices.end();
    bool res = stopApMode();
    return res;
}
bool  Esp3DNetwork::stopBtMode()
{
    esp3d_log("Stop BT Mode");
    return false;
}

bool Esp3DNetwork::setMode (esp3d_radio_mode_t mode, bool restart)
{
    esp3d_log("Current mode is %d, and ask for %d", _current_radio_mode, mode);

    if (mode == _current_radio_mode && !restart) {
        esp3d_log("Current mode and new mode are identical so cancel");
        return true;
    }

    switch(_current_radio_mode) {
    case esp3d_radio_off:
        stopNoRadioMode();
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
        startNoRadioMode();
        break;
    case esp3d_wifi_sta:
        if (!startStaMode()) {
            setMode((esp3d_radio_mode_t) esp3dTFTsettings.readByte(esp3d_fallback_mode));
        }
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