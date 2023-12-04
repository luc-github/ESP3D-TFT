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

#define COMMAND_ID 750
// Action on SD Filesystem
// rmdir / remove / mkdir / exists /create
//[ESP750]<Action>=<path> json=<no> pwd=<admin password>
void ESP3DCommands::ESP750(int cmd_params_pos, ESP3DMessage* msg) {
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
  const char* cmdList[] = {"rmdir=", "remove=", "mkdir=", "exists=", "create="};
  uint8_t cmdListSize = sizeof(cmdList) / sizeof(char*);
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  uint8_t i;
  for (i = 0; i < cmdListSize; i++) {
    tmpstr = get_param(msg, cmd_params_pos, cmdList[i]);
    if (tmpstr.length() != 0) {
      break;
    }
  }
  if (i >= cmdListSize || tmpstr.length() == 0) {
    hasError = true;
  } else {
    if (sd.accessFS()) {
      switch (i) {
        case 0:
          if (!sd.rmdir(tmpstr.c_str())) {
            hasError = true;
            error_msg = "rmdir failed";
          }
          break;
        case 1:
          if (!sd.remove(tmpstr.c_str())) {
            hasError = true;
            error_msg = "remove failed";
          }
          break;
        case 2:
          if (!sd.mkdir(tmpstr.c_str())) {
            hasError = true;
            error_msg = "mkdir failed";
          }
          break;
        case 3:
          if (sd.exists(tmpstr.c_str())) {
            ok_msg = "yes";
          } else {
            ok_msg = "no";
          }
          break;
        case 4: {
          FILE* fd = sd.open(tmpstr.c_str(), "w");
          if (fd) {
            sd.close(fd);
          } else {
            hasError = true;
            error_msg = "creation failed";
          }
        } break;
        default:
          hasError = true;
          break;
      }
      sd.releaseFS();
    } else {
      hasError = true;
      if (sd.getState() == ESP3DSdState::busy) {
        error_msg = "SD busy";
      } else {
        error_msg = "No SD";
      }
    }
  }

  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // #if ESP3D_SD_CARD_FEATURE