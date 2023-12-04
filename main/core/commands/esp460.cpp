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
#include "translations/esp3d_translation_service.h"

#define COMMAND_ID 460
// Get/Set UI language
// output is JSON or plain text according parameter
//[ESP460]DUMP json=<no> <pwd=admin/user> <language code>
void ESP3DCommands::ESP460(int cmd_params_pos, ESP3DMessage* msg) {
  ESP3DClientType target = msg->origin;
  ESP3DRequest requestId = msg->request_id;
  (void)requestId;
  msg->target = target;
  msg->origin = ESP3DClientType::command;
  bool hasError = false;
  std::string error_msg = "Invalid parameters";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  bool dump = hasTag(msg, cmd_params_pos, "DUMP");
  std::string tmpstr;
  char out_str[255] = {0};
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE
  if (dump) {
    tmpstr =
        "#Translation pack for <code> language\n#Name file: "
        "ui_<code>.lng\n[translations]\n";
    if (!dispatch(tmpstr.c_str(), target, requestId, ESP3DMessageType::head)) {
      esp3d_log_e("Error sending answer to clients");
    }
    for (uint16_t i = 0; i < (uint16_t)(ESP3DLabel::unknown_index); i++) {
      tmpstr = esp3dTranslationService.getEntry((ESP3DLabel)i);
      tmpstr += "=";
      tmpstr += esp3dTranslationService.translate((ESP3DLabel)i);
      tmpstr += "\n";
      if (!dispatch(tmpstr.c_str(), target, requestId,
                    ESP3DMessageType::core)) {
        esp3d_log_e("Error sending answer to clients");
      }
    }
    tmpstr =
        "\n[info]\ncreator=luc@tech-hunter.com\ncreation_date=2023-05-"
        "16\nmaintainer==luc@tech-hunter.com\nlast_update=2023-05-16\ntarget_"
        "language=<English name>\ntarget_version=3.0\n";

    if (!dispatch(tmpstr.c_str(), target, requestId, ESP3DMessageType::tail)) {
      esp3d_log_e("Error sending answer to clients");
    }
    return;
  }
  tmpstr = get_clean_param(msg, cmd_params_pos);
  if (tmpstr.length() == 0) {
    const ESP3DSettingDescription* settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_ui_language);
    if (settingPtr) {
      ok_msg = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_ui_language,
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
    if (esp3d_string::startsWith(tmpstr.c_str(), "DUMP=")) {
      hasError = true;
      error_msg = "Set value failed";
    } else {
      esp3d_log("got %s param for a value of %s, is valid %d", tmpstr.c_str(),
                tmpstr.c_str(),
                esp3dTftsettings.isValidStringSetting(
                    tmpstr.c_str(), ESP3DSettingIndex::esp3d_ui_language));
      if (esp3dTftsettings.isValidStringSetting(
              tmpstr.c_str(), ESP3DSettingIndex::esp3d_ui_language)) {
        esp3d_log("Value %s is valid", tmpstr.c_str());
        if (!esp3dTftsettings.writeString(ESP3DSettingIndex::esp3d_ui_language,
                                          tmpstr.c_str())) {
          hasError = true;
          error_msg = "Set value failed";
        } else {
          esp3dTranslationService.begin();
        }
      } else {
        hasError = true;
        error_msg = "Invalid parameter";
      }
    }
  }
  if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                      hasError ? error_msg.c_str() : ok_msg.c_str())) {
    esp3d_log_e("Error sending response to clients");
  }
}