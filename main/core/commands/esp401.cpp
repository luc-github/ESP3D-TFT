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
#if ESP3D_SD_CARD_FEATURE
#include "sd_def.h"
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE
#include "gcode_host/esp3d_tft_stream.h"
#include "notifications/esp3d_notifications_service.h"
#include "translations/esp3d_translation_service.h"

#define COMMAND_ID 401

// Set EEPROM setting
//[ESP401]P=<position> T=<type> V=<value> json=<no> pwd=<admin password>
void ESP3DCommands::ESP401(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;

  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  msg->type = ESP3DMessageType::unique;
  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";

#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level != ESP3DAuthenticationLevel::admin) {
    msg->authentication_level = ESP3DAuthenticationLevel::not_authenticated;
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE

  std::string settingIndex = get_param(msg, cmd_params_pos, "P=");
  std::string settingType = get_param(msg, cmd_params_pos, "T=");
  ;
  std::string settingValue = get_param(msg, cmd_params_pos, "V=");
  esp3d_log("Got P=%s T=%s V=%s", settingIndex.c_str(), settingType.c_str(),
            settingValue.c_str());
  // basic sanity check - do we get all parameters
  if (settingIndex.length() == 0 || settingType.length() != 1) {
    esp3d_log_e("Invalid parameter P or T");
    hasError = true;
  } else {
    // check parameters are correct
    uint8_t valueb = 0;
    uint32_t value32 = 0;
    ESP3DSettingIndex index_setting =
        (ESP3DSettingIndex)atoi(settingIndex.c_str());
    const ESP3DSettingDescription* settingPtr =
        esp3dTftsettings.getSettingPtr(index_setting);
    if (!settingPtr) {
      hasError = true;
      error_msg = "Unknown setting";
    } else {
      switch (settingType[0]) {
        case 'B':
          if (settingPtr->type == ESP3DSettingType::byte_t) {
            valueb = (uint8_t)atoi(settingValue.c_str());
            if (esp3dTftsettings.isValidByteSetting(valueb, index_setting)) {
              if (!esp3dTftsettings.writeByte(index_setting, valueb)) {
                hasError = true;
                error_msg = "Failed set value";
              }
            } else {
              hasError = true;
              error_msg = "Incorrect value";
            }
          } else {
            hasError = true;
            error_msg = "Incorrect type";
          }
          break;
        case 'I':
          if (settingPtr->type == ESP3DSettingType::integer_t) {
            value32 = (uint32_t)atoi(settingValue.c_str());
            if (esp3dTftsettings.isValidIntegerSetting(value32,
                                                       index_setting)) {
              if (!esp3dTftsettings.writeUint32(index_setting, value32)) {
                hasError = true;
                error_msg = "Failed set value";
              }
            } else {
              hasError = true;
              error_msg = "Incorrect value";
            }
          } else {
            hasError = true;
            error_msg = "Incorrect type";
          }
          break;
        case 'S':
          if (settingPtr->type == ESP3DSettingType::string_t) {
            if (esp3dTftsettings.isValidStringSetting(settingValue.c_str(),
                                                      index_setting)) {
              if (!esp3dTftsettings.writeString(index_setting,
                                                settingValue.c_str())) {
                hasError = true;
                error_msg = "Failed set value";
              }
            } else {
              hasError = true;
              error_msg = "Incorrect value";
            }
          } else {
            hasError = true;
            error_msg = "Incorrect type";
          }
          break;
        case 'A':

          if (settingPtr->type == ESP3DSettingType ::ip) {
            if (esp3dTftsettings.isValidIPStringSetting(settingValue.c_str(),
                                                        index_setting)) {
              if (!esp3dTftsettings.writeIPString(index_setting,
                                                  settingValue.c_str())) {
                hasError = true;
                error_msg = "Failed set value";
              }
            } else {
              hasError = true;
              error_msg = "Incorrect value";
            }
          } else {
            hasError = true;
            error_msg = "Incorrect type";
          }
          break;
        case 'M':
          // esp3d_mask;
          hasError = true;
          error_msg = "Not supported yet";
          break;
        case 'F':
          // esp3d_float;
          hasError = true;
          error_msg = "Not supported yet";
          break;
        case 'X':
          // esp3d_bitsfield;
          hasError = true;
          error_msg = "Not supported yet";
          break;
        default:
          hasError = true;
          error_msg = "Unknown type";
          break;
      }
    }
    // hot changes
    if (!hasError) {
      switch (index_setting) {
        case ESP3DSettingIndex::esp3d_ui_language:
          esp3dTranslationService.begin();
          break;
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_SD_IS_SPI
        case ESP3DSettingIndex::esp3d_spi_divider:
          sd.setSPISpeedDivider(valueb);
          break;
#endif  // ESP3D_SD_IS_SPI
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_AUTHENTICATION_FEATURE
        case ESP3DSettingIndex::esp3d_admin_password:
          esp3dAuthenthicationService.setAdminPassword(settingValue.c_str());
          break;
        case ESP3DSettingIndex::esp3d_user_password:
          esp3dAuthenthicationService.setUserPassword(settingValue.c_str());
          break;
        case ESP3DSettingIndex::esp3d_session_timeout:
          esp3dAuthenthicationService.setSessionTimeout(valueb);
          break;
#endif  // ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
        case ESP3DSettingIndex::esp3d_notification_type:
        case ESP3DSettingIndex::esp3d_auto_notification:
        case ESP3DSettingIndex::esp3d_notification_token_1:
        case ESP3DSettingIndex::esp3d_notification_token_2:
        case ESP3DSettingIndex::esp3d_notification_token_setting:
          esp3dNotificationsService.begin();
          break;
#endif  // ESP3D_NOTIFICATIONS_FEATURE

        case ESP3DSettingIndex::esp3d_target_firmware:
          esp3dTftstream.getTargetFirmware(true);
          break;
        default:
          break;
      }
    }
  }

  if (hasError) {
    if (json) {
      tmpstr = "{\"cmd\":\"401\",\"status\":\"error\",\"data\":{\"error\":\"";
      tmpstr += error_msg;
      tmpstr += "\",\"position\":\"";
      tmpstr += settingIndex;
      tmpstr += "\"}}";
    } else {
      tmpstr = error_msg + "\n";
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
  if (!dispatch(msg, tmpstr.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
