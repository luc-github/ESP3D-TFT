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
#include "esp3d-settings.h"
#include "authentication/esp3d_authentication.h"
#include "filesystem/esp3d_sd.h"
#define COMMAND_ID 401

//Set EEPROM setting
//[ESP401]P=<position> T=<type> V=<value> json=<no> pwd=<admin password>
void Esp3DCommands::ESP401(int cmd_params_pos,esp3d_msg_t * msg)
{
    esp3d_clients_t target = msg->origin;
    esp3d_request_t requestId = msg->requestId;

    (void)requestId;
    msg->target = target;
    msg->origin = ESP3D_COMMAND;
    bool json = hasTag (msg,cmd_params_pos,"json");
    std::string tmpstr;
    bool hasError = false;
    std::string error_msg ="Invalid parameters";

#if ESP3D_AUTHENTICATION_FEATURE
    if (msg->authentication_level != ESP3D_LEVEL_ADMIN) {
        msg->authentication_level =ESP3D_LEVEL_NOT_AUTHENTICATED;
        dispatchAuthenticationError(msg, COMMAND_ID,json);
        return;
    }
#endif //ESP3D_AUTHENTICATION_FEATURE

    std::string settingIndex = get_param (msg, cmd_params_pos,"P=");
    std::string settingType = get_param (msg, cmd_params_pos,"T=");;
    std::string settingValue = get_param (msg, cmd_params_pos,"V=");
    esp3d_log("Got P=%s T=%s V=%s",settingIndex.c_str(),settingType.c_str(),  settingValue.c_str());
    //basic sanity check - do we get all parameters
    if (settingIndex.length()==0 || settingValue.length()==0 || settingType.length()!=1) {
        esp3d_log("Got P=%d T=%d V=%d",settingIndex.length(),settingType.length(),  settingValue.length());
        hasError = true;
    } else {
        //check parameters are correct
        esp3d_setting_index_t index_setting = (esp3d_setting_index_t) atoi(settingIndex.c_str ());
        const Esp3DSetting_t * settingPtr= esp3dTFTsettings.getSettingPtr(index_setting);
        if (!settingPtr) {
            hasError = true;
            error_msg= "Unknown setting";
        } else {
            switch (settingType[0]) {
            case 'B':
                if (settingPtr->type == esp3d_byte) {
                    uint8_t value = (uint8_t)atoi(settingValue.c_str());
                    if (esp3dTFTsettings.isValidByteSetting(value, index_setting)) {
                        if (!esp3dTFTsettings.writeByte(index_setting, value)) {
                            hasError = true;
                            error_msg= "Failed set value";
                        } else {
                            //hot change setting if any
                            switch(index_setting) {
                            case esp3d_spi_divider:
                                sd.setSPISpeedDivider(value);
                                break;
                            default:
                                break;
                            }
                        }
                    } else {
                        hasError = true;
                        error_msg= "Incorrect value";
                    }
                } else {
                    hasError = true;
                    error_msg= "Incorrect type";
                }
                break;
            case 'I':
                if (settingPtr->type == esp3d_integer) {
                    uint32_t value = (uint32_t)atoi(settingValue.c_str());
                    if (esp3dTFTsettings.isValidIntegerSetting(value, index_setting)) {
                        if (!esp3dTFTsettings.writeUint32(index_setting, value)) {
                            hasError = true;
                            error_msg= "Failed set value";
                        } else {
                            //hot change setting if any
                            switch(index_setting) {
                            default:
                                break;
                            }
                        }
                    } else {
                        hasError = true;
                        error_msg= "Incorrect value";
                    }
                } else {
                    hasError = true;
                    error_msg= "Incorrect type";
                }
                break;
            case 'S':
                if (settingPtr->type == esp3d_string) {
                    if (esp3dTFTsettings.isValidStringSetting(settingValue.c_str(), index_setting)) {
                        if (!esp3dTFTsettings.writeString(index_setting, settingValue.c_str())) {
                            hasError = true;
                            error_msg= "Failed set value";
                        } else {
                            //hot change setting if any
                            switch(index_setting) {
                            default:
                                break;
                            }
                        }
                    } else {
                        hasError = true;
                        error_msg= "Incorrect value";
                    }
                } else {
                    hasError = true;
                    error_msg= "Incorrect type";
                }
                break;
            case 'A':

                if (settingPtr->type == esp3d_ip) {
                    if (esp3dTFTsettings.isValidIPStringSetting(settingValue.c_str(), index_setting)) {
                        if (!esp3dTFTsettings.writeIPString(index_setting, settingValue.c_str())) {
                            hasError = true;
                            error_msg= "Failed set value";
                        } else {
                            //hot change setting if any
                            switch(index_setting) {
                            default:
                                break;
                            }
                        }
                    } else {
                        hasError = true;
                        error_msg= "Incorrect value";
                    }
                } else {
                    hasError = true;
                    error_msg= "Incorrect type";
                }
                break;
            case 'M':
                // esp3d_mask;
                hasError = true;
                error_msg= "Not supported yet";
                break;
            case 'F':
                // esp3d_float;
                hasError = true;
                error_msg= "Not supported yet";
                break;
            case 'X':
                //esp3d_bitsfield;
                hasError = true;
                error_msg= "Not supported yet";
                break;
            default:
                hasError = true;
                error_msg= "Unknown type";
                break;
            }
        }
    }
    if (hasError) {
        if (json) {
            tmpstr = "{\"cmd\":\"401\",\"status\":\"error\",\"data\":{\"error\":\"";
            tmpstr+=error_msg;
            tmpstr +="\",\"position\":\"";
            tmpstr += settingIndex;
            tmpstr += "\"}}";
        } else {
            tmpstr = error_msg +"\n";
        }
    } else {
        if (json) {
            tmpstr = "{\"cmd\":\"401\",\"status\":\"ok\",\"data\":\"";
            tmpstr += settingIndex;
            tmpstr += "\"}";
        } else {
            tmpstr = "ok\n";
        }
    }
    if(!dispatch(msg,tmpstr.c_str())) {
        esp3d_log_e("Error sending response to clients");
    }
}