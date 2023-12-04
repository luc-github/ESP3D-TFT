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

#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_string.h"
#include "serial/esp3d_serial_client.h"

#define COMMAND_ID 900

// Get state / Set Enable / Disable Serial Communication
//[ESP900]<ENABLE/DISABLE> json=<no> pwd=<admin/user password>
void ESP3DCommands::ESP900(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool setEnable = hasTag(msg, cmd_params_pos, "ENABLE");
  bool setDisable = hasTag(msg, cmd_params_pos, "DISABLE");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  if (!(setEnable || setDisable) && !has_param(msg, cmd_params_pos)) {  // get
    hasError = true;
  } else {
    if (setEnable) {  // set
      if (setEnable) {
        if (!serialClient.started()) {
          if (!serialClient.begin()) {
            hasError = true;
            error_msg = "error processing command";
          }
        }
      }
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
    return;
  }
  if (setDisable) {
    if (serialClient.started()) {
      serialClient.end();
    }
  }
}