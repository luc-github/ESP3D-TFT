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

#pragma once
#include <stdio.h>
#include <string>
#include "esp3d_settings.h"
#include "notifications/customizations.h"
#include "esp_http_client.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum  {
    esp3d_no_notification,
    esp3d_pushover_notification,
    esp3d_email_notification,
    esp3d_line_notification,
    esp3d_telegram_notification,
    esp3d_ifttt_notification
} esp3d_notification_type_t;


class Esp3DNotificationsService final
{
public:
    Esp3DNotificationsService();
    ~Esp3DNotificationsService();
    bool begin();
    void handle();
    void end();
    bool sendMSG(const char * title, const char * message);
    bool sendPushoverMSG(const char * title, const char * message);
    bool sendEmailMSG(const char * title, const char * message);
    bool sendLineMSG(const char * title, const char * message);
    bool sendTelegramMSG(const char * title, const char * message);
    bool sendIFTTTMSG(const char * title, const char * message);
    const char * getTypeString();
    esp3d_notification_type_t getType()
    {
        return _notificationType;
    }
    bool started()
    {
        return _started;
    }
    bool isAutonotification()
    {
        return _autonotification;
    };
    void setAutonotification(bool value)
    {
        _autonotification = value;
    };
    bool sendAutoNotification(const char * msg);

private:
    bool _started;
    bool _autonotification;
    esp3d_notification_type_t _notificationType;
    std::string _token1;
    std::string _token2;
    std::string _settings;
    std::string _serveraddress;
    std::string _port;
    bool encodeBase64(const char *data, std::string *result);
    bool getEmailInformationsFromSettings();
};

extern Esp3DNotificationsService esp3dNotificationsService;
#ifdef __cplusplus
} // extern "C"
#endif