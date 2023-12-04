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
#if ESP3D_WEBDAV_SERVICES_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "socket_server/esp3d_socket_server.h"

#define COMMAND_ID 190
// Get/Set WebDav state which can be ON, OFF
//[ESP190]<state> json=<no> pwd=<admin password>
void ESP3DCommands::ESP190(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool stateON = hasTag(msg, cmd_params_pos, "ON");
  bool stateOFF = hasTag(msg, cmd_params_pos, "OFF");
  bool has_param = false;
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  ESP3DState setting_mode =
      (ESP3DState)esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_webdav_on);
  if (tmpstr.length() == 0) {
    if (setting_mode == ESP3DState::off) {
      ok_msg = "OFF";
    } else {
      ok_msg = "ON";
    }
  } else {
    if (stateON || stateOFF) {
      if (!esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_webdav_on,
                                      stateOFF ? 0 : 1)) {
        hasError = true;
        error_msg = "Set value failed";
      }
      has_param = true;
    }
    if (!has_param) {
      hasError = true;
      error_msg = "Invalid parameter";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}

#endif  // ESP3D_WEBDAV_SERVICES_FEATURE