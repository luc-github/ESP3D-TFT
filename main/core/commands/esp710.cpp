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
#include "filesystem/esp3d_flash.h"

#define COMMAND_ID 710

// Format ESP Filesystem
//[ESP710]FORMATFS json=<no> pwd=<admin password>
void ESP3DCommands::ESP710(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "Format successful";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool needFormat = hasTag(msg, cmd_params_pos, "FORMATFS");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  ESP3DMessage* endMsg = nullptr;
  if (needFormat) {
    ok_msg = "Starting formating...";
    endMsg = ESP3DClient::copyMsgInfos(*msg);
  } else {
    hasError = true;
  }
  if (!json || hasError) {  // cannot have multiple json messages so only keep
                            // last message or eeror message
    if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                        hasError ? error_msg.c_str() : ok_msg.c_str())) {
      esp3d_log_e("Error sending response to clients");
    }
  } else {
    ESP3DClient::deleteMsg(msg);
  }
  if (!hasError) {
    flashFs.accessFS();
    flush();
    if (!flashFs.format()) {
      hasError = true;
      error_msg = "Format failed";
    } else {
      ok_msg = "Formating done";
    }
    if (endMsg) {
      if (!dispatchAnswer(endMsg, COMMAND_ID, json, hasError,
                          hasError ? error_msg.c_str() : ok_msg.c_str())) {
        esp3d_log_e("Error sending response to clients");
      }
    }
    flashFs.releaseFS();
  }
}