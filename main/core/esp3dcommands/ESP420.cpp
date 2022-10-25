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

    //ESP3D-TFT-VERSION
    if (!dispatchIdValue(json,"FW Ver", ESP3D_TFT_VERSION, target,requestId, true)) {
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

    //Flash size
    tmpstr = formatBytes(spi_flash_get_chip_size());
    if (!dispatchIdValue(json,"flash size",tmpstr.c_str(), target,requestId)) {
        return;
    }

    //Free memory
    tmpstr =formatBytes(esp_get_minimum_free_heap_size());
    if (!dispatchIdValue(json,"free mem",tmpstr.c_str(), target,requestId)) {
        return;
    }
#if CONFIG_SPIRAM
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    tmpstr =formatBytes( info.total_free_bytes + info.total_allocated_bytes);
    if (!dispatchIdValue(json,"Total psram mem",tmpstr.c_str(), target, requestId)) {
        return;
    }
#endif //CONFIG_SPIRAM

    //end of list
    if (json) {
        if(!dispatch("]}",target,requestId)) {
            esp3d_log_e("Error sending answer to clients");
        }
    }

}