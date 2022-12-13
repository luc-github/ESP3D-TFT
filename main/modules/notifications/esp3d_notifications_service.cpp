/*
  esp3d_network
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
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "mbedtls/base64.h"
#include <cstdlib>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "websocket/esp3d_ws_service.h"

#define PUSHOVERTIMEOUT 5000
#define PUSHOVERSERVER "api.pushover.net"
#define PUSHOVERPORT    "443"

#define LINETIMEOUT 5000
#define LINESERVER "notify-api.line.me"
#define LINEPORT    "443"

#define TELEGRAMTIMEOUT 5000
#define TELEGRAMSERVER "api.telegram.org"
#define TELEGRAMPORT    "443"

#define IFTTTTIMEOUT 5000
#define IFTTTSERVER "maker.ifttt.com"
#define IFTTTPORT    "443"

#define EMAILTIMEOUT 5000

#define STEP_EMAIL 0
#define STEP_ADDRESS 1
#define STEP_PORT 2


Esp3DNotificationsService esp3dNotificationsService;

Esp3DNotificationsService::Esp3DNotificationsService()
{
    end();
}

bool Esp3DNotificationsService::encodeBase64(const char *data, std::string *result)
{
    bool res = false;
    char *dataencoded = (char *)malloc(2*strlen(data)+1);
    size_t finalLen = 0;
    if (!dataencoded) {
        esp3d_log_e("memory allocate failed");
        return false;
    }
    if (mbedtls_base64_encode( (unsigned char*)dataencoded, 2*strlen(data), &finalLen,
                               (const unsigned char*)data,strlen(data) )==0) {
        res = true;
    }
    *result = dataencoded;
    free(dataencoded);
    return res;
}

Esp3DNotificationsService::~Esp3DNotificationsService() {}
bool Esp3DNotificationsService::begin()
{
    end();
    bool res = true;
    char buffer[SIZE_OF_SETTING_NOFIFICATION_T1];
    end();
    _notificationType = (esp3d_notification_type_t)esp3dTFTsettings.readByte(esp3d_notification_type);
    switch(_notificationType) {
    case esp3d_no_notification: //no notification = no error but no start
        _started=true;
        return true;
    case esp3d_pushover_notification:
        _token1 = esp3dTFTsettings.readString(esp3d_notification_token_1, buffer, SIZE_OF_SETTING_NOFIFICATION_T1);
        _token2 = esp3dTFTsettings.readString(esp3d_notification_token_2, buffer, SIZE_OF_SETTING_NOFIFICATION_T2);
        _port = PUSHOVERPORT;
        _serveraddress = PUSHOVERSERVER;
        break;
    case esp3d_telegram_notification:
        _token1 = esp3dTFTsettings.readString(esp3d_notification_token_1, buffer, SIZE_OF_SETTING_NOFIFICATION_T1);
        _token2 = esp3dTFTsettings.readString(esp3d_notification_token_2, buffer, SIZE_OF_SETTING_NOFIFICATION_T2);
        _port = TELEGRAMPORT;
        _serveraddress = TELEGRAMSERVER;
        break;
    case esp3d_line_notification:
        _token1 = esp3dTFTsettings.readString(esp3d_notification_token_1, buffer, SIZE_OF_SETTING_NOFIFICATION_T1);
        _port = LINEPORT;
        _serveraddress = LINESERVER;
        break;
    case esp3d_ifttt_notification:
        _token1 = esp3dTFTsettings.readString(esp3d_notification_token_1, buffer, SIZE_OF_SETTING_NOFIFICATION_T1);
        _token2 = esp3dTFTsettings.readString(esp3d_notification_token_2, buffer, SIZE_OF_SETTING_NOFIFICATION_T2);
        _port = IFTTTPORT;
        _serveraddress = IFTTTSERVER;
        break;
    case esp3d_email_notification:
        if ( encodeBase64(esp3dTFTsettings.readString(esp3d_notification_token_1, buffer, SIZE_OF_SETTING_NOFIFICATION_T1), &_token1)) {
            if ( encodeBase64(esp3dTFTsettings.readString(esp3d_notification_token_2, buffer, SIZE_OF_SETTING_NOFIFICATION_T2), &_token2)) {
                if(!getEmailInformationsFromSettings()) {
                    res = false;
                }
            } else {
                res=false;
            }
        } else {
            res=false;
        }
        break;
    default:
        res = false;
        break;
    }

    if (!res) {
        end();
    } else {
        _autonotification = esp3dTFTsettings.readByte(esp3d_auto_notification);
    }
    _started = res;
    return _started;
}
void Esp3DNotificationsService::handle() {}
void Esp3DNotificationsService::end()
{
    _started = false;
    _autonotification=false;
    _notificationType = esp3d_no_notification;
    _token1.clear();
    _token2.clear();
    _settings.clear();
    _serveraddress.clear();
    _port.clear();

}
bool Esp3DNotificationsService::sendMSG(const char * title, const char * message)
{
    esp3dWsWebUiService.pushNotification(message);

//TODO according notification type
    return true;
}

const char * Esp3DNotificationsService::getTypeString()
{
    switch(_notificationType) {
    case esp3d_pushover_notification:
        return "pushover";
    case esp3d_email_notification:
        return "email";
    case esp3d_line_notification:
        return "line";
    case esp3d_telegram_notification:
        return "telegram";
    case esp3d_ifttt_notification:
        return "IFTTT";
    default:
        break;
    }
    return "none";
}

bool Esp3DNotificationsService::sendAutoNotification(const char * msg)
{
    return true;
}


//Email#serveraddress:port
bool Esp3DNotificationsService::getEmailInformationsFromSettings()
{
    char buffer[SIZE_OF_SETTING_NOFIFICATION_TS+1];
    esp3dTFTsettings.readString(esp3d_notification_token_setting, buffer, SIZE_OF_SETTING_NOFIFICATION_TS);
    //0 = email, 1 =server address, 2 = port
    uint8_t step = STEP_EMAIL;
    for(uint i = 0; i < strlen(buffer); i++) {
        if(buffer[i] ==':' || buffer[i] == '#') {
            step++;
        } else {
            switch (step) {
            case STEP_EMAIL:
                _settings += buffer[i];
                break;
            case STEP_ADDRESS:
                _serveraddress += buffer[i];
                break;
            case STEP_PORT:
                _port += buffer[i];
                break;
            default :
                return false;
            }
        }
    }
    if (step!=2) {
        esp3d_log_e("Missing some Parameters");
        return false;
    }
    esp3d_log("Server: %s, port: %s, email: %s", _serveraddress.c_str(),_port.c_str(), _settings.c_str());
    return true;
}