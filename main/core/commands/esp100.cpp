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

#define COMMAND_ID 100
// Set/Get STA SSID
// output is JSON or plain text according parameter
//[ESP100]<SSID> json=<no> pwd=<admin password for set/get & user password to
// get>
void ESP3DCommands::ESP100(int cmd_params_pos, ESP3DMessage* msg) {
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
  char out_str[255] = {0};
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    const ESP3DSettingDescription* settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_sta_ssid);
    if (settingPtr) {
      ok_msg = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_sta_ssid,
                                           out_str, settingPtr->size);
    } else {
      hasError = true;
      error_msg = "This setting is unknown";
    }
  } else {
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // ESP3D_AUTHENTICATION_FEATURE
    esp3d_log("got %s param for a value of %s, is valid %d", tmpstr.c_str(),
              tmpstr.c_str(),
              esp3dTftsettings.isValidStringSetting(
                  tmpstr.c_str(), ESP3DSettingIndex::esp3d_sta_ssid));
    if (esp3dTftsettings.isValidStringSetting(
            tmpstr.c_str(), ESP3DSettingIndex::esp3d_sta_ssid)) {
      esp3d_log("Value %s is valid", tmpstr.c_str());
      if (!esp3dTftsettings.writeString(ESP3DSettingIndex::esp3d_sta_ssid,
                                        tmpstr.c_str())) {
        hasError = true;
        error_msg = "Set value failed";
      }
    } else {
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