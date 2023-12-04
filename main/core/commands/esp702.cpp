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
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "gcode_host/esp3d_gcode_host_service.h"

#define COMMAND_ID 702
// Define scripts used when script is paused/aborted/resumed
//[ESP702]<pause/stop/resume>=<script> - display/set ESP700 stream scripts
void ESP3DCommands::ESP702(int cmd_params_pos, ESP3DMessage* msg) {
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
  const char* cmdList[] = {"pause=", "stop=", "resume="};
  uint8_t cmdListSize = sizeof(cmdList) / sizeof(char*);
  const ESP3DSettingIndex settingIndex[] = {
      ESP3DSettingIndex::esp3d_pause_script,
      ESP3DSettingIndex::esp3d_stop_script,
      ESP3DSettingIndex::esp3d_resume_script};
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    char buffer[SIZE_OF_SCRIPT + 1];
    if (json) {
      ok_msg = "{";
    } else {
      ok_msg = "";
    }
    for (uint8_t i = 0; i < cmdListSize; i++) {
      if (json) {
        if (i != 0) {
          ok_msg += ",";
        }

        ok_msg += "\"";
        std::string tmp = cmdList[i];
        ok_msg += tmp.substr(0, tmp.length() - 1);
        ok_msg += "_script";
        ok_msg += "\":\"";
      } else {
        ok_msg += cmdList[i];
      }
      ok_msg +=
          esp3dTftsettings.readString(settingIndex[i], buffer, SIZE_OF_SCRIPT);
      if (json) {
        ok_msg += "\"";
        if (i == cmdListSize - 1) {
          ok_msg += "}";
        }
      } else {
        ok_msg += "\n";
      }
    }

  } else {
    bool hasParam = false;
    for (uint8_t i = 0; i < cmdListSize; i++) {
      bool flag_present = false;
      tmpstr = get_param(msg, cmd_params_pos, cmdList[i], &flag_present);
      if (tmpstr.length() != 0 || flag_present) {
        hasParam = true;
        if (esp3dTftsettings.isValidStringSetting(tmpstr.c_str(),
                                                  settingIndex[i])) {
          esp3d_log("Value %s is valid", tmpstr.c_str());
          if (!esp3dTftsettings.writeString(settingIndex[i], tmpstr.c_str())) {
            hasError = true;
            error_msg = "Set value failed";
          }
        } else {
          hasError = true;
          error_msg = "Invalid token parameter";
        }
      }
    }
    if (!hasParam && !hasError) {
      hasError = true;
      error_msg = "Invalid parameter";
    }
    if (!hasError) {
      gcodeHostService.updateScripts();
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}