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
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "authentication/esp3d_authentication.h"
#define COMMAND_ID 444

//Set ESP State
//`cmd` can be  `RESTART` to restart board or `RESET` to reset all setting to  defaults values
//[ESP444]<cmd> json=<no> <pwd=admin>`
void Esp3DCommands::ESP444(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    (void)requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;
    bool json = hasTag (msg,cmd_params_pos,"json");
    std::string tmpstr;
    bool isRestart = hasTag (msg, cmd_params_pos,"RESTART");
    bool isReset = hasTag (msg, cmd_params_pos,"RESET");
    bool hasError = false;
    std::string error_msg ="Invalid parameters";
    std::string ok_msg ="";
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level != ESP3D_LEVEL_ADMIN) {
        msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
        dispatchAuthenticationError(msg, COMMAND_ID, json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE
    if (isReset) {
        esp3dTFTsettings.reset();
        esp3d_log("Resetting settings");
    }

    if (!(isRestart||isReset)) {
        hasError= true;
    } else {

        if (isReset) {
            ok_msg += "Reset done";
        }
        if (isRestart) {
            if (ok_msg.length()>0 ) {
                ok_msg += ", ";
            }
            ok_msg += "Now restarting...";
        }
    }
    if(!dispatchAnswer(msg,COMMAND_ID, json, hasError, hasError?error_msg.c_str():ok_msg.c_str())) {
        esp3d_log_e("Error sending response to clients");
        return;
    }
    if (isRestart) {
        flush();
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
        while(1) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}