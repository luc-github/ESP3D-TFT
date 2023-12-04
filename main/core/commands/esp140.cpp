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
#if ESP3D_TIMESTAMP_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "time/esp3d_time_service.h"

#define COMMAND_ID 140

// Sync / Set / Get current time
//[ESP140]<sync> <srv1=XXXXX> <srv2=XXXXX> <srv3=XXXXX> <tzone=xxx>
//<time=YYYY-MM-DDTHH:mm:ss> <ntp=YES/NO> <now> json=<no> pwd=<admin password>
void ESP3DCommands::ESP140(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  // use the longuest string to avoid buffer overflow
  char buffer[SIZE_OF_SERVER_URL + 1];
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool sync = hasTag(msg, cmd_params_pos, "sync");
  bool now = hasTag(msg, cmd_params_pos, "now");
  std::string tmpstr;
  const char* cmdList[] = {"srv1", "srv2", "srv3", "tzone", "ntp", "time"};
  uint8_t cmdListSize = sizeof(cmdList) / sizeof(char*);
  const ESP3DSettingIndex settingIndex[] = {
      ESP3DSettingIndex::esp3d_time_server1,
      ESP3DSettingIndex::esp3d_time_server2,
      ESP3DSettingIndex::esp3d_time_server3,
      ESP3DSettingIndex::esp3d_timezone,
      ESP3DSettingIndex::esp3d_use_internet_time,
      ESP3DSettingIndex::unknown_index,
  };
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    if (json) {
      ok_msg = "{";
    } else {
      ok_msg = "";
    }
    // no need to show time right now
    for (uint i = 0; i < cmdListSize - 2; i++) {
      if (json) {
        if (i > 0) {
          ok_msg += ",";
        }
        ok_msg += "\"";
      } else {
        if (i > 0) {
          ok_msg += ", ";
        }
      }
      ok_msg += cmdList[i];

      if (json) {
        ok_msg += "\":\"";
      } else {
        ok_msg += ": ";
      }
      ok_msg += esp3dTftsettings.readString(settingIndex[i], buffer,
                                            SIZE_OF_SERVER_URL);
      if (json) {
        ok_msg += "\"";
      } else {
      }
    }
    if (json) {
      ok_msg += ",\"ntp\":\"";
    } else {
      ok_msg += ", ntp: ";
    }
    ok_msg +=
        esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_use_internet_time)
            ? "yes"
            : "no";
    if (json) {
      ok_msg += "\"";
    } else {
    }

    if (json) {
      ok_msg += "}";
    } else {
      ok_msg += "\n";
    }
  } else {
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // ESP3D_AUTHENTICATION_FEATURE    bool hasParam = false;
    bool hasParam = false;
    std::string tmpkey;
    for (uint8_t i = 0; i < cmdListSize; i++) {
      tmpkey = cmdList[i];
      tmpkey += "=";
      tmpstr = get_param(msg, cmd_params_pos, tmpkey.c_str());
      if (tmpstr.length() != 0) {
        hasParam = true;
        if (settingIndex[i] == ESP3DSettingIndex::unknown_index) {
          // set time
          if (!esp3dTimeService.setTime(tmpstr.c_str())) {
            hasError = true;
            error_msg = "Set time failed";
          }
        } else if (settingIndex[i] ==
                   ESP3DSettingIndex::esp3d_use_internet_time) {
          esp3d_string::str_toLowerCase(&tmpstr);
          if (tmpstr == "yes" || tmpstr == "no" || tmpstr == "1" ||
              tmpstr == "0" || tmpstr == "true" || tmpstr == "false") {
            uint8_t val = 0;
            if (tmpstr == "yes" || tmpstr == "1" || tmpstr == "true") {
              val = 1;
            }
            if (!esp3dTftsettings.writeByte(settingIndex[i], val)) {
              hasError = true;
              error_msg = "Set value failed";
            }
          } else {
            hasError = true;
            error_msg = "Invalid token parameter";
          }

        } else {
          if (esp3dTftsettings.isValidStringSetting(tmpstr.c_str(),
                                                    settingIndex[i])) {
            esp3d_log("Value %s is valid", tmpstr.c_str());
            if (!esp3dTftsettings.writeString(settingIndex[i],
                                              tmpstr.c_str())) {
              hasError = true;
              error_msg = "Set value failed";
            }
          } else {
            hasError = true;
            error_msg = "Invalid token parameter";
          }
        }
      }
    }
    if (!hasError && now) {
      ok_msg = esp3dTimeService.getCurrentTime();
      ok_msg += "  (";
      ok_msg += esp3dTimeService.getTimeZone();
      ok_msg += ")";
      hasParam = true;
    }
    if (!hasError && sync) {
      // apply changes without restarting the board
      esp3dTimeService.begin();
      hasParam = true;
    }
    if (!hasParam && !hasError) {
      hasError = true;
      error_msg = "Invalid parameter";
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // ESP3D_TIMESTAMP_FEATURE