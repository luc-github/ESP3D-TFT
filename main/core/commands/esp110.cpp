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
#include "network/esp3d_network.h"

#define COMMAND_ID 110
// Set radio state at boot which can be BT, CONFIG STA, AP, OFF
//[ESP110]<state>  json=<no> pwd=<admin password>
void ESP3DCommands::ESP110(int cmd_params_pos, ESP3DMessage* msg) {
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
    byteValue = esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_radio_mode);
    if (byteValue == (uint8_t)ESP3DRadioMode::bluetooth_serial) {
      ok_msg = "BT";
    } else
#if ESP3D_WIFI_FEATURE
        if (byteValue == (uint8_t)ESP3DRadioMode::wifi_ap) {
      ok_msg = "AP";
    } else if (byteValue == (uint8_t)ESP3DRadioMode::wifi_sta) {
      ok_msg = "STA";
    } else if (byteValue == (uint8_t)ESP3DRadioMode::wifi_ap_config) {
      ok_msg = "CONFIG";
    } else
#endif  // ESP3D_WIFI_FEATURE
      if (byteValue == (uint8_t)ESP3DRadioMode::off) {
        ok_msg = "OFF";
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
    if (tmpstr == "BT") {
      byteValue = (uint8_t)ESP3DRadioMode::bluetooth_serial;
    } else
#if ESP3D_WIFI_FEATURE
        if (tmpstr == "AP") {
      byteValue = (uint8_t)ESP3DRadioMode::wifi_ap;
    } else if (tmpstr == "STA") {
      byteValue = (uint8_t)ESP3DRadioMode::wifi_sta;
    } else if (tmpstr == "CONFIG") {
      byteValue = (uint8_t)ESP3DRadioMode::wifi_ap_config;
    } else
#endif  // ESP3D_WIFI_FEATURE
      if (tmpstr == "OFF") {
        byteValue = (uint8_t)ESP3DRadioMode::off;
      } else {
        byteValue = (uint8_t)-1;  // unknow flag so put outof range value
      }
    esp3d_log("got %s param for a value of %d, is valid %d", tmpstr.c_str(),
              byteValue,
              esp3dTftsettings.isValidByteSetting(
                  byteValue, ESP3DSettingIndex::esp3d_radio_mode));
    if (esp3dTftsettings.isValidByteSetting(
            byteValue, ESP3DSettingIndex::esp3d_radio_mode)) {
      esp3d_log("Value %d is valid", byteValue);
      if (!esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_radio_mode,
                                      byteValue)) {
        hasError = true;
        error_msg = "Set value failed";
      } else {
        esp3dNetwork.setMode(static_cast<ESP3DRadioMode>(byteValue));
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