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
#include "esp3d_settings.h"
#include "authentication/esp3d_authentication.h"

//Set EEPROM setting
//[ESP401]P=<position> T=<type> V=<value> json=<no> pwd=<admin password>
void Esp3DCommands::ESP100(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    (void)requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;

    bool json = hasTag (msg,cmd_params_pos,"json");
    std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level != ESP3D_LEVEL_ADMIN) {
        msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
        dispatchAuthenticationError(msg, 100,json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE
    if (json) {
        tmpstr = "{\"cmd\":\"100\",\"status\":\"ok\",\"data\":[]}";
        if(!dispatch(msg,tmpstr.c_str())) {
            esp3d_log_e("Error sending response to clients");
            return;
        }
    } else {
        // Esp3DClient::deleteMsg(msg);
        if(!dispatch(msg,"ok")) {
            esp3d_log_e("Error sending response to clients");
            return;
        }
    }
}