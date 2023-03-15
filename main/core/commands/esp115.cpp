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
#include "network/esp3d_network.h"

#define COMMAND_ID 115
// Get/Set immediate Network (WiFi/BT/Ethernet) state which can be ON, OFF
//[ESP115]<state> json=<no> pwd=<admin password>
void Esp3DCommands::ESP115(int cmd_params_pos, Esp3dMessage* msg) {
  Esp3dClient target = msg->origin;
  esp3d_request_t requestId = msg->requestId;
  (void)requestId;
  msg->target = target;
  msg->origin = Esp3dClient::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == Esp3dAuthenticationLevel::guest) {
    msg->authentication_level = Esp3dAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  Esp3dRadioMode setting_radio_mode =
      static_cast<Esp3dRadioMode>(esp3dTFTsettings.readByte(esp3d_radio_mode));
  Esp3dRadioMode current_radio_mode = esp3dNetwork.getMode();
  if (tmpstr.length() == 0) {
    if (current_radio_mode == Esp3dRadioMode::off) {
      ok_msg = "OFF";
    } else {
      ok_msg = "ON";
    }
  } else {
    if (tmpstr == "OFF") {
      if (current_radio_mode != Esp3dRadioMode::off) {
        if (!esp3dNetwork.setMode(Esp3dRadioMode::off)) {
          hasError = true;
          error_msg = "Fail to stop network";
        }
      }
    } else if (tmpstr == "ON") {
      if (current_radio_mode == Esp3dRadioMode::off) {
        if (!esp3dNetwork.setMode(setting_radio_mode)) {
          hasError = true;
          error_msg = "Fail to start network";
        }
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