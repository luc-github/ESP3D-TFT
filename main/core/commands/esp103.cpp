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
#if ESP3D_WIFI_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"

#define COMMAND_ID 103
// Change STA IP/Mask/GW/DNS
//[ESP103]IP=<IP> MSK=<IP> GW=<IP> DNS=<IP> json=no pwd=<admin password>
void ESP3DCommands::ESP103(int cmd_params_pos, ESP3DMessage* msg) {
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
  const char* cmdList[] = {"IP=", "MSK=", "GW=", "DNS="};
  uint8_t cmdListSize = sizeof(cmdList) / sizeof(char*);
  const ESP3DSettingIndex settingIndex[] = {
      ESP3DSettingIndex::esp3d_sta_ip_static,
      ESP3DSettingIndex::esp3d_sta_mask_static,
      ESP3DSettingIndex::esp3d_sta_gw_static,
      ESP3DSettingIndex::esp3d_sta_dns_static};
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    if (json) {
      ok_msg = "{\"ip\":\"";
    } else {
      ok_msg = "IP: ";
    }
    ok_msg +=
        esp3dTftsettings.readIPString(ESP3DSettingIndex::esp3d_sta_ip_static);
    if (json) {
      ok_msg += "\",\"gw\":\"";
    } else {
      ok_msg += ", GW: ";
    }
    ok_msg +=
        esp3dTftsettings.readIPString(ESP3DSettingIndex::esp3d_sta_gw_static);
    if (json) {
      ok_msg += "\",\"msk\":\"";
    } else {
      ok_msg += ", MSK: ";
    }
    ok_msg +=
        esp3dTftsettings.readIPString(ESP3DSettingIndex::esp3d_sta_mask_static);
    if (json) {
      ok_msg += "\",\"dns\":\"";
    } else {
      ok_msg += ", DNS: ";
    }
    ok_msg +=
        esp3dTftsettings.readIPString(ESP3DSettingIndex::esp3d_sta_dns_static);
    if (json) {
      ok_msg += "\"}";
    } else {
      ok_msg += "\n";
    }
  } else {
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // ESP3D_AUTHENTICATION_FEATURE
    bool hasParam = false;
    for (uint8_t i = 0; i < cmdListSize; i++) {
      tmpstr = get_param(msg, cmd_params_pos, cmdList[i]);
      if (tmpstr.length() != 0) {
        hasParam = true;
        if (esp3dTftsettings.isValidIPStringSetting(tmpstr.c_str(),
                                                    settingIndex[i])) {
          esp3d_log("Value %s is valid", tmpstr.c_str());
          if (!esp3dTftsettings.writeIPString(settingIndex[i],
                                              tmpstr.c_str())) {
            hasError = true;
            error_msg = "Set value failed";
          }
        } else {
          hasError = true;
          error_msg = "Invalid parameter";
        }
      }
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
#endif  // ESP3D_WIFI_FEATURE