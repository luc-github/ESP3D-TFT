/*
  esp3d_notifications_service
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

#include "esp3d_notifications_service.h"

#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_values.h"
#include "mbedtls/base64.h"
#include "network/esp3d_network.h"

#if ESP3D_HTTP_FEATURE
#include "websocket/esp3d_webui_service.h"
#endif  // ESP3D_HTTP_FEATURE

ESP3DNotificationsService esp3dNotificationsService;

ESP3DNotificationsService::ESP3DNotificationsService() { end(); }

ESP3DNotificationsService::~ESP3DNotificationsService() {}

bool ESP3DNotificationsService::begin(bool sendAutoNotificationMsg) {
  esp3d_log("Start notification service");
  bool res = true;
  char buffer[SIZE_OF_SETTING_NOFIFICATION_T1];
  end();
  _notificationType = (ESP3DNotificationType)esp3dTftsettings.readByte(
      ESP3DSettingIndex::esp3d_notification_type);
  switch (_notificationType) {
    case ESP3DNotificationType::none:  // no notification = no error but no
                                       // start
      _started = true;
      return true;
    case ESP3DNotificationType::pushover:
      _token1 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      _token2 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_2, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T2);
      break;
    case ESP3DNotificationType::telegram:
      _token1 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      _token2 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_2, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T2);
      break;
    case ESP3DNotificationType::line:
      _token1 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      break;
    case ESP3DNotificationType::ifttt:
      _token1 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      _token2 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_2, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T2);
      break;
    case ESP3DNotificationType::email:
      _token1 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      _token2 = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_notification_token_2, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T2);
      if (!getEmailInformationsFromSettings()) {
        res = false;
      }
      break;
    default:
      res = false;
      break;
  }

  if (!res) {
    esp3d_log_e("Failed to start notification service");
    end();
  } else {
    _autonotification =
        esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_auto_notification);
  }
  _started = res;
  if (sendAutoNotificationMsg) {
    sendAutoNotification(ESP3D_NOTIFICATION_ONLINE);
  }
  return _started;
}
void ESP3DNotificationsService::handle() {}

void ESP3DNotificationsService::end() {
  _started = false;
  _autonotification = false;
  _notificationType = ESP3DNotificationType::none;
  _token1.clear();
  _token2.clear();
  _settings.clear();
  _serveraddress.clear();
  _port.clear();
  _method.clear();
  _lastError = ESP3DNotificationError::no_error;
}

bool ESP3DNotificationsService::sendMSG(const char* title,
                                        const char* message) {
  std::string formated_message;
  std::string formated_title;
  formated_message = esp3d_string::expandString(message);
  if (formated_message.length() == 0) {
    _lastError = ESP3DNotificationError::empty_message;
    esp3d_log_e("Empty notification message");
    return false;
  }
  formated_title = esp3d_string::expandString(title);
  if (formated_title.length() == 0) {
    formated_title = "Notification";
  }
#if ESP3D_HTTP_FEATURE
  esp3dWsWebUiService.pushNotification(formated_message.c_str());
#endif  // ESP3D_HTTP_FEATURE
  esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                  formated_message.c_str());

  if (_started) {
    switch (_notificationType) {
      case ESP3DNotificationType::pushover:
        return sendPushoverMSG(formated_title.c_str(),
                               formated_message.c_str());
        break;
      case ESP3DNotificationType::telegram:
        return sendTelegramMSG(formated_title.c_str(),
                               formated_message.c_str());
        break;
      case ESP3DNotificationType::line:
        return sendLineMSG(formated_title.c_str(), formated_message.c_str());
        break;
      case ESP3DNotificationType::ifttt:
        return sendIFTTTMSG(formated_title.c_str(), formated_message.c_str());
        break;
      case ESP3DNotificationType::email:
        return sendEmailMSG(formated_title.c_str(), formated_message.c_str());
        break;
      default:
        break;
    }
  }
  return true;
}

const char* ESP3DNotificationsService::getTypeString() {
  switch (_notificationType) {
    case ESP3DNotificationType::pushover:
      return "pushover";
    case ESP3DNotificationType::email:
      return "email";
    case ESP3DNotificationType::line:
      return "line";
    case ESP3DNotificationType::telegram:
      return "telegram";
    case ESP3DNotificationType::ifttt:
      return "IFTTT";
    default:
      break;
  }
  return "none";
}

bool ESP3DNotificationsService::sendAutoNotification(const char* msg) {
  if (_autonotification && _started) {
    return sendMSG(ESP3D_NOTIFICATION_TITLE, msg);
  }
  return false;
}
