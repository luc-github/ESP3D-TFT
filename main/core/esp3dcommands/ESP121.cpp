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
#if ESP3D_HTTP_FEATURE
#include "esp3d_commands.h"
#include "esp3d_client.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "authentication/esp3d_authentication.h"
#include "network/esp3d_network.h"
#define COMMAND_ID 121
//Get/Set HTTP port
//[ESP121]<port> json=<no> pwd=<admin password>
void Esp3DCommands::ESP121(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    (void)requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;
    bool hasError = false;
    std::string error_msg ="Invalid parameters";
    std::string ok_msg ="ok";
    bool json = hasTag (msg,cmd_params_pos,"json");
    std::string tmpstr;
    uint32_t intValue =0;
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level == ESP3D_LEVEL_GUEST) {
        msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
        dispatchAuthenticationError(msg, COMMAND_ID,json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE
    tmpstr = get_clean_param(msg,cmd_params_pos);
    if (tmpstr.length()==0) {
        intValue = esp3dTFTsettings.readUint32(esp3d_http_port);
        ok_msg=std::to_string(intValue);
    } else {
        intValue = atoi(tmpstr.c_str());
        esp3d_log("got %s param for a value of %ld, is valid %d", tmpstr.c_str(),intValue, esp3dTFTsettings.isValidIntegerSetting(intValue, esp3d_http_port));
        if (esp3dTFTsettings.isValidIntegerSetting(intValue, esp3d_http_port)) {
            esp3d_log("Value %ld is valid",intValue);
            if (!esp3dTFTsettings.writeUint32 (esp3d_http_port, intValue)) {
                hasError = true;
                error_msg="Set value failed";
            }
        } else {
            hasError=true;
            error_msg="Invalid parameter";
        }
    }

    if(!dispatchAnswer(msg,COMMAND_ID, json, hasError, hasError?error_msg.c_str():ok_msg.c_str())) {
        esp3d_log_e("Error sending response to clients");
    }
}
#endif //ESP3D_HTTP_FEATURE
