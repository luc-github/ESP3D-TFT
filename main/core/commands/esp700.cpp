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
#include "gcode_host/esp3d_gcode_host_service.h"

#define COMMAND_ID 700

// Read / Stream  / Process FS file
//[ESP700]<filename> json=<no> pwd=<admin/user password>
void ESP3DCommands::ESP700(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    hasError = true;
    error_msg = "Missing parameter";
    esp3d_log_e("Error missing");
  } else {
    bool isMacro = true;
    std::string filename = get_param(msg, cmd_params_pos, "stream=");
    if (filename.length() > 0) {  // it is a stream
      isMacro = false;
    } else {  // it is a macro
      filename = get_clean_param(msg, cmd_params_pos);
    }
    esp3d_log("Stream: %s", filename.c_str());
    if (gcodeHostService.addStream(filename.c_str(), msg->authentication_level,
                                   isMacro)) {
      esp3d_log("Stream: %s added as %s", filename.c_str(),
                isMacro ? "Macro" : "File");
    } else {
      hasError = false;
      error_msg = "Failed to add stream";
      esp3d_log_e("Failed to add stream");
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
