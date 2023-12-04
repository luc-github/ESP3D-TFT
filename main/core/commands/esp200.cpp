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
#if ESP3D_SD_CARD_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_string.h"
#include "filesystem/esp3d_sd.h"

#define COMMAND_ID 200
// Get SD Card Status
//[ESP200]<RELEASE> <REFRESH> pwd=<user/admin password>
void ESP3DCommands::ESP200(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool isRelease = hasTag(msg, cmd_params_pos, "RELEASE");
  bool isRefresh = hasTag(msg, cmd_params_pos, "REFRESH");
  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  if (isRelease) {
    sd.releaseFS();
    ok_msg = "SD card released";
  }
  if (isRefresh) {
    if (!sd.getSpaceInfo(nullptr, nullptr, nullptr, true)) {
      hasError = true;
      error_msg = "Refresh failed";
    }
  }
  if (!isRelease && !isRefresh) {
    tmpstr = get_clean_param(msg, cmd_params_pos);
    if (tmpstr.length() != 0) {
      hasError = true;
    } else {
      ESP3DSdState state = sd.getState();
      switch (state) {
        case ESP3DSdState::idle:
          ok_msg = "SD card ok";
          break;
        case ESP3DSdState::not_present:
          ok_msg = "No SD card";
          break;
        case ESP3DSdState::busy:
          ok_msg = "Busy";
          break;
        default:
          ok_msg = "Unknow";
      }
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // ESP3D_SD_CARD_FEATURE
