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
#include "mbedtls/base64.h"
#include "network/esp3d_network.h"

#if ESP3D_HTTP_FEATURE
#include "websocket/esp3d_webui_service.h"
#endif  // ESP3D_HTTP_FEATURE

ESP3dNotificationsService esp3dNotificationsService;

ESP3dNotificationsService::ESP3dNotificationsService() { end(); }

ESP3dNotificationsService::~ESP3dNotificationsService() {}

bool ESP3dNotificationsService::begin(bool sendAutoNotificationMsg) {
  esp3d_log("Start notification service");
  bool res = true;
  char buffer[SIZE_OF_SETTING_NOFIFICATION_T1];
  end();
  _notificationType = (Esp3dNotificationType)esp3dTFTsettings.readByte(
      Esp3dSettingIndex::esp3d_notification_type);
  switch (_notificationType) {
    case Esp3dNotificationType::none:  // no notification = no error but no
                                       // start
      _started = true;
      return true;
    case Esp3dNotificationType::pushover:
      _token1 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      _token2 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_2, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T2);
      break;
    case Esp3dNotificationType::telegram:
      _token1 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      _token2 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_2, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T2);
      break;
    case Esp3dNotificationType::line:
      _token1 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      break;
    case Esp3dNotificationType::ifttt:
      _token1 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      _token2 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_2, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T2);
      break;
    case Esp3dNotificationType::email:
      _token1 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_1, buffer,
          SIZE_OF_SETTING_NOFIFICATION_T1);
      _token2 = esp3dTFTsettings.readString(
          Esp3dSettingIndex::esp3d_notification_token_2, buffer,
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
        esp3dTFTsettings.readByte(Esp3dSettingIndex::esp3d_auto_notification);
  }
  _started = res;
  if (sendAutoNotificationMsg) {
    sendAutoNotification(ESP3D_NOTIFICATION_ONLINE);
  }
  return _started;
}
void ESP3dNotificationsService::handle() {}

void ESP3dNotificationsService::end() {
  _started = false;
  _autonotification = false;
  _notificationType = Esp3dNotificationType::none;
  _token1.clear();
  _token2.clear();
  _settings.clear();
  _serveraddress.clear();
  _port.clear();
  _method.clear();
  _lastError = Esp3dNotificationError::no_error;
}

bool ESP3dNotificationsService::sendMSG(const char* title,
                                        const char* message) {
  std::string formated_message = message;
  std::string formated_title = title;

  if (formated_message.find('%') != std::string::npos) {
    formated_message = esp3d_strings::str_replace(
        formated_message.c_str(), "%ESP_IP%", esp3dNetwork.getLocalIpString());
    formated_message = esp3d_strings::str_replace(
        formated_message.c_str(), "%ESP_NAME%", esp3dNetwork.getHostName());
  }
  if (formated_message.length() == 0) {
    _lastError = Esp3dNotificationError::empty_message;
    esp3d_log_e("Empty notification message");
    return false;
  }
  if (formated_title.find('%') != std::string::npos) {
    formated_title = esp3d_strings::str_replace(
        formated_message.c_str(), "%ESP_IP%", esp3dNetwork.getLocalIpString());
    formated_title = esp3d_strings::str_replace(
        formated_message.c_str(), "%ESP_NAME%", esp3dNetwork.getHostName());
  }
  if (formated_title.length() == 0) {
    formated_title = "Notification";
  }
#if ESP3D_HTTP_FEATURE
  esp3dWsWebUiService.pushNotification(formated_message.c_str());
#endif  // ESP3D_HTTP_FEATURE

  if (_started) {
    switch (_notificationType) {
      case Esp3dNotificationType::pushover:
        return sendPushoverMSG(formated_title.c_str(),
                               formated_message.c_str());
        break;
      case Esp3dNotificationType::telegram:
        return sendTelegramMSG(formated_title.c_str(),
                               formated_message.c_str());
        break;
      case Esp3dNotificationType::line:
        return sendLineMSG(formated_title.c_str(), formated_message.c_str());
        break;
      case Esp3dNotificationType::ifttt:
        return sendIFTTTMSG(formated_title.c_str(), formated_message.c_str());
        break;
      case Esp3dNotificationType::email:
        return sendEmailMSG(formated_title.c_str(), formated_message.c_str());
        break;
      default:
        break;
    }
  }
  return true;
}

const char* ESP3dNotificationsService::getTypeString() {
  switch (_notificationType) {
    case Esp3dNotificationType::pushover:
      return "pushover";
    case Esp3dNotificationType::email:
      return "email";
    case Esp3dNotificationType::line:
      return "line";
    case Esp3dNotificationType::telegram:
      return "telegram";
    case Esp3dNotificationType::ifttt:
      return "IFTTT";
    default:
      break;
  }
  return "none";
}

bool ESP3dNotificationsService::sendAutoNotification(const char* msg) {
  if (_autonotification && _started) {
    return sendMSG(ESP3D_NOTIFICATION_TITLE, msg);
  }
  return false;
}
