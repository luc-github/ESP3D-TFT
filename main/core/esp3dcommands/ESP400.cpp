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
#include "esp3d_string.h"
#include "authentication/esp3d_authentication.h"

const char * BaudRateList[] = {"9600", "19200", "38400", "57600", "74880", "115200", "230400", "250000", "500000", "921600"};

//Get full ESP3D settings
//[ESP400]<pwd=admin>
void Esp3DCommands::ESP400(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;

    bool json = hasTag (msg,cmd_params_pos,"json");
    esp3d_log("Size is %d", sizeof(BaudRateList));
    std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level == ESP3D_LEVEL_GUEST) {
        msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
        dispatchAuthenticationError(msg, 100,json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE
    if (json) {
        tmpstr = "{\"cmd\":\"400\",\"status\":\"ok\",\"data\":[";
        if(!dispatch(msg,tmpstr.c_str())) {
            esp3d_log_e("Error sending response to clients");
            return;
        }
    } else {
        Esp3DClient::deleteMsg(msg);
    }
    if (!dispatchSetting(json,"system/system",esp3d_baud_rate, "baud", BaudRateList, BaudRateList, sizeof(BaudRateList)/sizeof(char*), -1, -1,-1, nullptr, true,target,requestId,true)) {
        esp3d_log_e("Error sending response to clients");
    }


    if (json) {
        if(!dispatch("]}",target, requestId)) {
            esp3d_log_e("Error sending response to clients");
        }
    }
}