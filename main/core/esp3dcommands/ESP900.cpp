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
#include "serial/esp3d_serial_client.h"

//Get state / Set Enable / Disable Serial Communication
//[ESP900]<ENABLE/DISABLE> json=<no> pwd=<admin/user password>
void Esp3DCommands::ESP900(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    (void)requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;

    bool json = hasTag (msg,cmd_params_pos,"json");
    bool setEnable = hasTag (msg, cmd_params_pos,"ENABLE");
    bool setDisable = hasTag (msg, cmd_params_pos,"DISABLE");
    std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level == ESP3D_LEVEL_GUEST) {
        msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
        dispatchAuthenticationError(msg, 100,json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE

    if (!(setEnable || setDisable) && !has_param(msg,cmd_params_pos)) { //get
        if (json) {
            tmpstr = "{\"cmd\":\"900\",\"status\":\"ok\",\"data\":\"";
        }
        tmpstr+= serialClient.started()?"ENABLE":"DISABLE";
        if(json) {
            tmpstr+= "Invalid parameters\"}";
        } else {
            tmpstr+= "\n";
        }
    } else {
        if (setEnable || setDisable) { //set
            bool error = false;
            if (setEnable) {
                if (!serialClient.started()) {
                    error = !serialClient.begin();
                }
            }
            if (json) {
                if (error) {
                    tmpstr = "{\"cmd\":\"900\",\"status\":\"error\",\"data\":\"error processing command\"}";
                } else {
                    tmpstr = "{\"cmd\":\"900\",\"status\":\"ok\",\"data\":\"ok\"}";
                }
            } else {
                if (error) {
                    tmpstr = "error processing command\n";
                } else {
                    tmpstr = "ok\n";
                }
            }
        } else {//invalid param
            if (json) {
                tmpstr = "{\"cmd\":\"900\",\"status\":\"error\",\"data\":\"Invalid parameters\"}";
            } else {
                tmpstr = "Invalid parameters\n";
            }
        }
    }
    if(!dispatch(msg,tmpstr.c_str())) {
        esp3d_log_e("Error sending response to clients");
        return;
    }
    if (setDisable) {
        if (serialClient.started()) {
            serialClient.end();
        }
    }
}