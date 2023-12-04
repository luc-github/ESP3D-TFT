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
#if ESP3D_NOTIFICATIONS_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "notifications/esp3d_notifications_service.h"

#define COMMAND_ID 610

// Set/Get Notification settings
//[ESP610]type=<NONE/PUSHOVER/EMAIL/LINE/IFTTT> T1=<token1> T2=<token2>
// TS=<Settings> json=<no> pwd=<admin password> Get will give type and settings
// only, not the protected T1/T2
void ESP3DCommands::ESP610(int cmd_params_pos, ESP3DMessage* msg) {
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
  const char* cmdList[] = {"type=", "T1=", "T2=", "TS=", "AUTO="};
  uint8_t cmdListSize = sizeof(cmdList) / sizeof(char*);
  const ESP3DSettingIndex settingIndex[] = {
      ESP3DSettingIndex::esp3d_notification_type,
      ESP3DSettingIndex::esp3d_notification_token_1,
      ESP3DSettingIndex::esp3d_notification_token_2,
      ESP3DSettingIndex::esp3d_notification_token_setting,
      ESP3DSettingIndex::esp3d_auto_notification};
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    if (json) {
      ok_msg = "{\"type\":\"";
    } else {
      ok_msg = "type: ";
    }
    uint8_t b =
        esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_notification_type);
    switch ((ESP3DNotificationType)b) {
      case ESP3DNotificationType::none:
        ok_msg += "NONE";
        break;
      case ESP3DNotificationType::pushover:
        ok_msg += "PUSHOVER";
        break;
      case ESP3DNotificationType::email:
        ok_msg += "EMAIL";
        break;
      case ESP3DNotificationType::line:
        ok_msg += "LINE";
        break;
      case ESP3DNotificationType::telegram:
        ok_msg += "TELEGRAM";
        break;
      case ESP3DNotificationType::ifttt:
        ok_msg += "IFTTT";
        break;
      default:
        ok_msg += "UNKNOWN";
    }
    if (json) {
      ok_msg += "\",\"AUTO\":\"";
    } else {
      ok_msg += ", AUTO: ";
    }

    ok_msg += esp3dTftsettings.readByte(
                  ESP3DSettingIndex::esp3d_auto_notification) == 1
                  ? "YES"
                  : "NO";
    char buffer[SIZE_OF_SETTING_NOFIFICATION_TS + 1];
    if (json) {
      ok_msg += "\",\"TS\":\"";
    } else {
      ok_msg += ", TS: ";
    }
    ok_msg += esp3dTftsettings.readString(
        ESP3DSettingIndex::esp3d_notification_token_setting, buffer,
        SIZE_OF_SETTING_NOFIFICATION_TS);
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
    uint8_t val;
    for (uint8_t i = 0; i < cmdListSize; i++) {
      tmpstr = get_param(msg, cmd_params_pos, cmdList[i]);
      if (tmpstr.length() != 0) {
        hasParam = true;
        switch (settingIndex[i]) {
          case ESP3DSettingIndex::esp3d_notification_type:
            if (strcasecmp(tmpstr.c_str(), "NONE") == 0) {
              val = static_cast<uint8_t>(ESP3DNotificationType::none);
            } else if (strcasecmp(tmpstr.c_str(), "PUSHOVER") == 0) {
              val = static_cast<uint8_t>(ESP3DNotificationType::pushover);
            } else if (strcasecmp(tmpstr.c_str(), "LINE") == 0) {
              val = static_cast<uint8_t>(ESP3DNotificationType::line);
            } else if (strcasecmp(tmpstr.c_str(), "EMAIL") == 0) {
              val = static_cast<uint8_t>(ESP3DNotificationType::email);
            } else if (strcasecmp(tmpstr.c_str(), "IFTTT") == 0) {
              val = static_cast<uint8_t>(ESP3DNotificationType::ifttt);
            } else if (strcasecmp(tmpstr.c_str(), "TELEGRAM") == 0) {
              val = static_cast<uint8_t>(ESP3DNotificationType::telegram);
            } else {
              hasError = true;
              error_msg = "Invalid type parameter";
            }
            if (!hasError) {
              if (!esp3dTftsettings.writeByte(settingIndex[i], val)) {
                hasError = true;
                error_msg = "Set value failed";
              }
            }
            break;
          case ESP3DSettingIndex::esp3d_notification_token_1:
          case ESP3DSettingIndex::esp3d_notification_token_2:
          case ESP3DSettingIndex::esp3d_notification_token_setting:
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
            break;
          case ESP3DSettingIndex::esp3d_auto_notification:
            if (strcasecmp(tmpstr.c_str(), "YES") == 0 ||
                strcasecmp(tmpstr.c_str(), "1") == 0 ||
                strcasecmp(tmpstr.c_str(), "ON") == 0 ||
                strcasecmp(tmpstr.c_str(), "ENABLED") == 0) {
              val = 1;
            } else if (strcasecmp(tmpstr.c_str(), "NO") == 0 ||
                       strcasecmp(tmpstr.c_str(), "0") == 0 ||
                       strcasecmp(tmpstr.c_str(), "OFF") == 0 ||
                       strcasecmp(tmpstr.c_str(), "DISABLED") == 0) {
              val = 0;
            } else {
              hasError = true;
              error_msg = "Invalid auto parameter";
            }
            if (!hasError) {
              if (!esp3dTftsettings.writeByte(settingIndex[i], val)) {
                hasError = true;
                error_msg = "Set value failed";
              }
            }
            break;
          default:
            hasError = true;
            error_msg = "Invalid parameter";
        }
      }
    }
    if (!hasParam && !hasError) {
      hasError = true;
      error_msg = "Invalid parameter";
    }
    if (!hasError) {
      // apply changes without restarting the board
      esp3dNotificationsService.begin();
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}
#endif  // ESP3D_NOTIFICATIONS_FEATURE