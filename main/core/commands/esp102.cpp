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
#include "network/esp3d_network.h"

#define COMMAND_ID 102
// Change STA IP mode (DHCP/STATIC)
//[ESP102]<mode> json=no pwd=<admin password>
void ESP3DCommands::ESP102(int cmd_params_pos, ESP3DMessage* msg) {
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
  uint8_t byteValue = (uint8_t)-1;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    byteValue = esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_sta_ip_mode);
    if (byteValue == static_cast<uint8_t>(ESP3DIpMode::dhcp)) {
      ok_msg = "DHCP";
    } else if (byteValue == static_cast<uint8_t>(ESP3DIpMode::staticIp)) {
      ok_msg = "STATIC";
    } else {
      ok_msg = "Unknown";
    }
  } else {
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // ESP3D_AUTHENTICATION_FEATURE
    if (tmpstr == "DHCP") {
      byteValue = static_cast<uint8_t>(ESP3DIpMode::dhcp);
    } else if (tmpstr == "STATIC") {
      byteValue = static_cast<uint8_t>(ESP3DIpMode::staticIp);
    } else {
      byteValue = (uint8_t)-1;  // unknow flag so put outof range value
    }
    esp3d_log("got %s param for a value of %d, is valid %d", tmpstr.c_str(),
              byteValue,
              esp3dTftsettings.isValidByteSetting(
                  byteValue, ESP3DSettingIndex::esp3d_sta_ip_mode));
    if (esp3dTftsettings.isValidByteSetting(
            byteValue, ESP3DSettingIndex::esp3d_sta_ip_mode)) {
      esp3d_log("Value %d is valid", byteValue);
      if (!esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_sta_ip_mode,
                                      byteValue)) {
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