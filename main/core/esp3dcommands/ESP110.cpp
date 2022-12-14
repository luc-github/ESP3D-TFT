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

#include "esp3d_commands.h"
#include "esp3d_client.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "authentication/esp3d_authentication.h"
#include "network/esp3d_network.h"
#define COMMAND_ID 110
//Set radio state at boot which can be BT, CONFIG STA, AP, OFF
//[ESP110]<state>  json=<no> pwd=<admin password>
void Esp3DCommands::ESP110(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;
    (void)requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;
    bool hasError = false;
    std::string error_msg ="Invalid parameters";
    std::string ok_msg ="ok";
    bool json = hasTag (msg,cmd_params_pos,"json");
    std::string tmpstr;
    uint8_t byteValue =(uint8_t)-1;
#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level == ESP3D_LEVEL_GUEST) {
        msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
        dispatchAuthenticationError(msg, COMMAND_ID,json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE
    tmpstr = get_clean_param(msg,cmd_params_pos);
    if (tmpstr.length()==0) {
        byteValue = esp3dTFTsettings.readByte(esp3d_radio_mode);
        if (byteValue==(uint8_t)esp3d_bluetooth_serial) {
            ok_msg="BT";
        } else  if (byteValue==(uint8_t)esp3d_wifi_ap) {
            ok_msg="AP";
        } else  if (byteValue==(uint8_t)esp3d_wifi_sta) {
            ok_msg="STA";
        } else if (byteValue==(uint8_t)esp3d_wifi_ap_config) {
            ok_msg="CONFIG";
        }  else if (byteValue==(uint8_t)esp3d_radio_off) {
            ok_msg="OFF";
        } else {
            ok_msg="Unknown";
        }
    } else {
        if (tmpstr=="BT") {
            byteValue=(uint8_t)esp3d_bluetooth_serial;
        } else if (tmpstr=="AP") {
            byteValue=(uint8_t)esp3d_wifi_ap;
        } else if (tmpstr=="STA") {
            byteValue=(uint8_t)esp3d_wifi_sta;
        } else if (tmpstr=="CONFIG") {
            byteValue=(uint8_t)esp3d_wifi_ap_config;
        } else if (tmpstr=="OFF") {
            byteValue=(uint8_t)esp3d_radio_off;
        } else {
            byteValue=(uint8_t)-1; //unknow flag so put outof range value
        }
        esp3d_log("got %s param for a value of %d, is valid %d", tmpstr.c_str(),byteValue, esp3dTFTsettings.isValidByteSetting(byteValue, esp3d_radio_mode));
        if (esp3dTFTsettings.isValidByteSetting(byteValue, esp3d_radio_mode)) {
            esp3d_log("Value %d is valid",byteValue);
            if (!esp3dTFTsettings.writeByte (esp3d_radio_mode, byteValue)) {
                hasError = true;
                error_msg="Set value failed";
            } else {
                esp3dNetwork.setMode ((esp3d_radio_mode_t)byteValue);
            }
        } else {
            hasError=true;
            error_msg="Invalid parameter";
        }
    }

    if(!dispatchAnswer(msg,COMMAND_ID, json, hasError, hasError?error_msg.c_str():ok_msg.c_str())) {
        esp3d_log_e("Error sending response to clients");
    }
}