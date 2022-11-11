/*
  esp3d_commands member
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

#include "esp3d_commands.h"
#include "esp3d_client.h"
#include "esp3d_version.h"
#include "esp3d_string.h"
#include <stdio.h>
#include <string>
#include <cstring>
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_spi_flash.h"
#include "authentication/esp3d_authentication.h"
#include "filesystem/esp3d_flash.h"
#include "network/esp3d_network.h"
#define COMMAND_ID 420

//Get ESP current status
//output is JSON or plain text according parameter
//[ESP420]json=<no>
void Esp3DCommands::ESP420(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;
    bool json = hasTag (msg,cmd_params_pos,"json");
    std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level == ESP3D_LEVEL_GUEST) {
        dispatchAuthenticationError(msg, COMMAND_ID, json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE
    if (json) {
        tmpstr = "{\"cmd\":\"420\",\"status\":\"ok\",\"data\":[";
        if(!dispatch(msg,tmpstr.c_str())) {
            esp3d_log_e("Error sending response to clients");
            return;
        }
    } else {
        Esp3DClient::deleteMsg(msg);
    }

//Screen
    if (!dispatchIdValue(json,"Screen", TFT_TARGET, target, requestId, true)) {
        return;
    }

    //ESP3D-TFT-VERSION
    if (!dispatchIdValue(json,"FW Ver", ESP3D_TFT_VERSION, target,requestId)) {
        return;
    }

    //FW Arch
    if (!dispatchIdValue(json,"FW arch", CONFIG_IDF_TARGET, target, requestId)) {
        return;
    }

    //SDK Version
    if (!dispatchIdValue(json,"SDK", IDF_VER, target,requestId)) {
        return;
    }

    //CPU Freq
    tmpstr= std::to_string(ets_get_cpu_frequency()) + "MHz";
    if (!dispatchIdValue(json,"CPU Freq",tmpstr.c_str(), target,requestId)) {
        return;
    }

    //Free memory
    tmpstr =esp3d_strings::formatBytes(esp_get_minimum_free_heap_size());
    if (!dispatchIdValue(json,"free mem",tmpstr.c_str(), target,requestId)) {
        return;
    }

#if CONFIG_SPIRAM
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    tmpstr =esp3d_strings::formatBytes( info.total_free_bytes + info.total_allocated_bytes);
    if (!dispatchIdValue(json,"Total psram mem",tmpstr.c_str(), target, requestId)) {
        return;
    }
#endif //CONFIG_SPIRAM
    //Flash size
    tmpstr = esp3d_strings::formatBytes(spi_flash_get_chip_size());
    if (!dispatchIdValue(json,"flash size",tmpstr.c_str(), target,requestId)) {
        return;
    }
    //FileSystem
    size_t totalBytes=0;
    size_t usedBytes=0;
    flashFs.getSpaceInfo(&totalBytes,&usedBytes);
    tmpstr = esp3d_strings::formatBytes(usedBytes);
    tmpstr+="/";
    tmpstr = esp3d_strings::formatBytes(totalBytes);
    if (!dispatchIdValue(json,"FS usage",tmpstr.c_str(), target,requestId)) {
        return;
    }

    //wifi
    if (esp3dNetwork.getMode()==esp3d_radio_off || esp3dNetwork.getMode()==esp3d_bluetooth_serial) {
        tmpstr="OFF";
    } else {
        tmpstr="ON";
    }
    if (!dispatchIdValue(json,"wifi",tmpstr.c_str(), target,requestId)) {
        return;
    }

    //hostname
    const Esp3DSetting_t * settingPtr = esp3dTFTsettings.getSettingPtr(esp3d_hostname);
    if (settingPtr) {
        char out_str[(settingPtr->size)+1]= {0};
        tmpstr = esp3dTFTsettings.readString(esp3d_hostname,out_str, settingPtr->size);
    } else {
        tmpstr="Error!!";
    }
    if (!dispatchIdValue(json,"hostname",tmpstr.c_str(), target,requestId)) {
        return;
    }
    //sta
    if (esp3dNetwork.getMode()==esp3d_wifi_sta) {
        tmpstr="ON";
    } else {
        tmpstr="OFF";
    }
    if (!dispatchIdValue(json,"sta",tmpstr.c_str(), target,requestId)) {
        return;
    }

    //Sta MAC
    tmpstr =esp3dNetwork.getSTAMac();
    if (!dispatchIdValue(json,"mac",tmpstr.c_str(), target,requestId)) {
        return;
    }

    if (esp3dNetwork.getMode()==esp3d_wifi_sta) {

        wifi_ap_record_t ap;
        esp_err_t res = esp_wifi_sta_get_ap_info(&ap);
        if (res == ESP_OK) {
            //ssid
            tmpstr=(const char *)ap.ssid;
            if (!dispatchIdValue(json,"SSID",tmpstr.c_str(), target,requestId)) {
                return;
            }
            //%signal
            int32_t signal = esp3dNetwork.getSignal(ap.rssi,false);
            tmpstr = std::to_string(signal);
            tmpstr +="%";
            if (!dispatchIdValue(json,"signal",tmpstr.c_str(), target,requestId)) {
                return;
            }
            tmpstr = std::to_string(ap.primary);
            if (!dispatchIdValue(json,"channel",tmpstr.c_str(), target,requestId)) {
                return;
            }
        } else {
            if (!dispatchIdValue(json,"status","not connected", target,requestId)) {
                return;
            }
        }

        if (esp3dNetwork.useStaticIp()) {
            tmpstr = "static";
        } else {
            tmpstr = "dhcp";
        }
        if (!dispatchIdValue(json,"ip mode",tmpstr.c_str(), target,requestId)) {
            return;
        }
        esp3d_ip_info_t  ipInfo;
        if (esp3dNetwork.getLocalIp(&ipInfo)) {
            tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.ip));
            if (!dispatchIdValue(json,"ip",tmpstr.c_str(), target,requestId)) {
                return;
            }
            tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.gw));
            if (!dispatchIdValue(json,"gw",tmpstr.c_str(), target,requestId)) {
                return;
            }
            tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.netmask));
            if (!dispatchIdValue(json,"msk",tmpstr.c_str(), target,requestId)) {
                return;
            }
            tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.dns_info.ip.u_addr.ip4));
            if (!dispatchIdValue(json,"DNS",tmpstr.c_str(), target,requestId)) {
                return;
            }
        }
    }
    //ap
    if (esp3dNetwork.getMode()==esp3d_wifi_ap || esp3dNetwork.getMode()==esp3d_wifi_ap_config) {
        tmpstr="ON";
    } else {
        tmpstr="OFF";
    }
    if (!dispatchIdValue(json,"ap",tmpstr.c_str(), target,requestId)) {
        return;
    }
    if (esp3dNetwork.getMode()==esp3d_wifi_ap_config) {

        if (!dispatchIdValue(json,"config","ON", target,requestId)) {
            return;
        }
    }
    //AP MAC
    tmpstr =esp3dNetwork.getAPMac();
    if (!dispatchIdValue(json,"mac(AP)",tmpstr.c_str(), target,requestId)) {
        return;
    }
    if (esp3dNetwork.getMode()==esp3d_wifi_ap || esp3dNetwork.getMode()==esp3d_wifi_ap_config) {
        wifi_config_t wconfig;
        esp_err_t res = esp_wifi_get_config(WIFI_IF_AP, &wconfig);
        if (res == ESP_OK) {
            //ssid
            tmpstr=(const char *)wconfig.ap.ssid;
            if (!dispatchIdValue(json,"SSID",tmpstr.c_str(), target,requestId)) {
                return;
            }

            tmpstr = std::to_string(wconfig.ap.channel);
            if (!dispatchIdValue(json,"channel",tmpstr.c_str(), target,requestId)) {
                return;
            }

        }
        esp3d_ip_info_t  ipInfo;
        if (esp3dNetwork.getLocalIp(&ipInfo)) {
            tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.ip));
            if (!dispatchIdValue(json,"ip",tmpstr.c_str(), target,requestId)) {
                return;
            }
            tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.gw));
            if (!dispatchIdValue(json,"gw",tmpstr.c_str(), target,requestId)) {
                return;
            }
            tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.netmask));
            if (!dispatchIdValue(json,"msk",tmpstr.c_str(), target,requestId)) {
                return;
            }
            tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.dns_info.ip.u_addr.ip4));
            if (!dispatchIdValue(json,"DNS",tmpstr.c_str(), target,requestId)) {
                return;
            }
        }
        wifi_sta_list_t sta_list;
        tcpip_adapter_sta_list_t tcpip_sta_list;
        ESP_ERROR_CHECK( esp_wifi_ap_get_sta_list(&sta_list));
        if (sta_list.num>0) {
            ESP_ERROR_CHECK( tcpip_adapter_get_sta_list (&sta_list, &tcpip_sta_list));
        }


        tmpstr = std::to_string(sta_list.num);

        if (!dispatchIdValue(json,"clients",tmpstr.c_str(), target,requestId)) {
            return;
        }
        for(int i = 0; i < sta_list.num; i++) {
            //Print the mac address of the connected station

            tmpstr = ip4addr_ntoa((const ip4_addr_t*)&(tcpip_sta_list.sta[i].ip));
            tmpstr +="(";
            tmpstr += esp3dNetwork.getMacAddress(sta_list.sta[i].mac);
            tmpstr +=")";
            std::string client = "client ";
            client+=std::to_string(i+1);
            if (!dispatchIdValue(json,client.c_str(),tmpstr.c_str(), target,requestId)) {
                return;
            }
        }
    }
//bt
    if (esp3dNetwork.getMode()==esp3d_bluetooth_serial ) {
        tmpstr="ON";
    } else {
        tmpstr="OFF";
    }
    if ( !dispatchIdValue(json,"bt",tmpstr.c_str(), target,requestId)) {
        return;
    }
    //BT MAC
    tmpstr =esp3dNetwork.getBTMac();
    if (!dispatchIdValue(json,"mac(BT)",tmpstr.c_str(), target,requestId)) {
        return;
    }

    //end of list
    if (json) {
        if(!dispatch("]}",target,requestId)) {
            esp3d_log_e("Error sending answer to clients");
        }
    }

}