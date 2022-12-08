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
#include "spi_flash_mmap.h"
#include "authentication/esp3d_authentication.h"
#include "filesystem/esp3d_flash.h"
#include "network/esp3d_network.h"
#include "update/esp3d_update_service.h"
#define COMMAND_ID 800

//get fw version firmare target and fw version
//eventually set time with pc time
//output is JSON or plain text according parameter
//[ESP800]json=<no> <time=YYYY-MM-DDTHH:mm:ss> <version=3.0.0-a11> <setup=0/1>
void Esp3DCommands::ESP800(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;
    std::string timeparam = get_param (msg, cmd_params_pos,"time=");
    std::string setupparam = get_param (msg, cmd_params_pos,"setup=");
    bool json = hasTag (msg,cmd_params_pos,"json");
    std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level == ESP3D_LEVEL_GUEST) {
        dispatchAuthenticationError(msg, COMMAND_ID, json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE
    if (timeparam.length()>0) {
        //TODO: init time parameter
    }
    if (setupparam.length()>0) {
        if (!esp3dTFTsettings.writeByte (esp3d_setup, setupparam=="1"?1:0)) {
            //not blocking error
            esp3d_log_e("Error writing setup state");
        }
    }
    if (json) {
        tmpstr = "{\"cmd\":\"800\",\"status\":\"ok\",\"data\":{";

    } else {
        tmpstr = "Capabilities:\n";
    }
    msg->type=msg_head;
    if(!dispatch(msg,tmpstr.c_str())) {
        esp3d_log_e("Error sending response to clients");
        return;
    }
    //ESP3D-TFT-VERSION
    if (!dispatchKeyValue(json,"FWVersion", ESP3D_TFT_VERSION, target,requestId,false,true)) {
        return;
    }
    uint8_t b = esp3dTFTsettings.readByte(esp3d_target_firmware);
    //FWTarget
    tmpstr = esp3dTFTsettings.GetFirmwareTargetShortName((esp3d_target_firmware_index_t)b);
    if (!dispatchKeyValue(json,"FWTarget", tmpstr.c_str(), target,requestId)) {
        return;
    }
    //FWTargetID
    tmpstr = std::to_string(b);
    if (!dispatchKeyValue(json,"FWTargetID", tmpstr.c_str(), target,requestId)) {
        return;
    }

    //Setup
    tmpstr = esp3dTFTsettings.readByte(esp3d_setup)==1?"Enabled":"Disabled";
    if (!dispatchKeyValue(json,"Setup", tmpstr.c_str(), target,requestId)) {
        return;
    }

    //SD connection
    if (!dispatchKeyValue(json,"SDConnection", "direct", target,requestId)) {
        return;
    }

    //Serial Protocol
    if (!dispatchKeyValue(json,"SerialProtocol", "Raw", target,requestId)) {
        return;
    }

    //Authentication
#if ESP3D_AUTHENTICATION_FEATURE
    tmpstr = "Enabled";
#else
    tmpstr = "Disabled";
#endif //ESP3D_AUTHENTICATION_FEATURE
    if (!dispatchKeyValue(json,"Authentication", tmpstr.c_str(), target,requestId)) {
        return;
    }
    //WebCommunication
    if (!dispatchKeyValue(json,"WebCommunication", "Asynchronous", target,requestId)) {
        return;
    }

    //WebSocketIP
    esp3d_ip_info_t  ipInfo;
    if (esp3dNetwork.getLocalIp(&ipInfo)) {
        tmpstr =  ip4addr_ntoa((const ip4_addr_t*)&(ipInfo.ip_info.ip));
        if (!dispatchKeyValue(json,"WebSocketIP",tmpstr.c_str(), target,requestId)) {
            return;
        }
        uint32_t intValue = esp3dTFTsettings.readUint32(esp3d_http_port);
        tmpstr=std::to_string(intValue);
        if (!dispatchKeyValue(json,"WebSocketPort",tmpstr.c_str(), target,requestId)) {
            return;
        }
    }
    //Hostname
    const esp3d_setting_desc_t * settingPtr = esp3dTFTsettings.getSettingPtr(esp3d_hostname);
    if (settingPtr) {
        char out_str[(settingPtr->size)+1]= {0};
        tmpstr = esp3dTFTsettings.readString(esp3d_hostname,out_str, settingPtr->size);
    } else {
        tmpstr="Error!!";
    }
    if (!dispatchKeyValue(json,"Hostname",tmpstr.c_str(), target,requestId)) {
        return;
    }

    //WiFiMode
    switch (esp3dNetwork.getMode()) {
    case esp3d_wifi_sta:
        tmpstr = "STA";
        break;
    case esp3d_wifi_ap:
        tmpstr = "AP";
        break;
    case esp3d_wifi_ap_config:
        tmpstr = "CONFIG";
        break;
    case esp3d_radio_off:
        tmpstr = "RADIO OFF";
        break;
    case esp3d_bluetooth_serial:
        tmpstr = "BT";
        break;
    default:
        tmpstr = "Unknown";
    }
    if (!dispatchKeyValue(json,"RadioMode", tmpstr.c_str(), target,requestId)) {
        return;
    }
    //WebUpdate
    if (!dispatchKeyValue(json,"WebUpdate", esp3dUpdateService.canUpdate()?"Enabled":"Disabled", target,requestId)) {
        return;
    }
    //FlashFileSystem
    if (!dispatchKeyValue(json,"FlashFileSystem", "LittleFS", target,requestId)) {
        return;
    }
    //HostPath
    if (!dispatchKeyValue(json,"HostPath", "/", target,requestId)) {
        return;
    }

    //Screen
    if (!dispatchKeyValue(json,"Screen", TFT_TARGET, target,requestId)) {
        return;
    }
    //TODO: update once setup ready
    //Time
    if (!dispatchKeyValue(json,"Time", "none", target,requestId)) {
        return;
    }
    //end of list
    if (json) {
        if(!dispatch("}}",target,requestId, msg_tail)) {
            esp3d_log_e("Error sending answer to clients");
        }
    } else {
        {
            if(!dispatch("ok\n",target,requestId, msg_tail)) {
                esp3d_log_e("Error sending answer to clients");
            }
        }
    }

}