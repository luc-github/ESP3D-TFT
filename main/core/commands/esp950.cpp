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
#if ESP3D_USB_SERIAL_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "serial/esp3d_serial_client.h"
#include "usb_serial/esp3d_usb_serial_client.h"

#define COMMAND_ID 950
// Get / Set Serial Output // Only used in ESP3D-TFT with board having the
// feature
//[ESP950]<SERIAL/USB> json=<no> pwd=<admin/user password>
void ESP3DCommands::ESP950(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool usbclient = hasTag(msg, cmd_params_pos, "USB");
  bool serialclient = hasTag(msg, cmd_params_pos, "SERIAL");
  bool has_param = false;
  std::string tmpstr;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  ESP3DClientType client_output = (ESP3DClientType)esp3dTftsettings.readByte(
      ESP3DSettingIndex::esp3d_output_client);
  if (tmpstr.length() == 0) {
    if (client_output == ESP3DClientType::usb_serial) {
      ok_msg = "USB";
    } else {
      ok_msg = "SERIAL";
    }
  } else {
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
      dispatchAuthenticationError(msg, COMMAND_ID, json);
      return;
    }
#endif  // ESP3D_AUTHENTICATION_FEATURE
    if ((usbclient && !serialclient) || (serialclient && !usbclient)) {
      ESP3DClientType newoutput =
          serialclient ? ESP3DClientType::serial : ESP3DClientType::usb_serial;
      if (!esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_output_client,
                                      static_cast<uint8_t>(newoutput))) {
        hasError = true;
        error_msg = "Set value failed";
      }  // hot change not yet supported

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

#endif  // #if ESP3D_USB_SERIAL_FEATURE