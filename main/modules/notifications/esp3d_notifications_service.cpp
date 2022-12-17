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
#include "esp_timer.h"
#include "network/esp3d_network.h"
#include "websocket/esp3d_ws_service.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"


#define PUSHOVERTIMEOUT 5000
#define PUSHOVERSERVER "https://api.pushover.net"
#define PUSHOVERPORT    "443"

#define LINETIMEOUT 5000
#define LINESERVER "https://notify-api.line.me"
#define LINEPORT    "443"

#define TELEGRAMTIMEOUT 5000
#define TELEGRAMSERVER "https://api.telegram.org"
#define TELEGRAMPORT    "443"

#define IFTTTTIMEOUT 5000
#define IFTTTSERVER "https://maker.ifttt.com"
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

bool Esp3DNotificationsService::begin(bool sendAutoNotificationMsg)
{
    esp3d_log("Start notification service");
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
        esp3d_log_e("Failed to start notification service");
        end();
    } else {
        _autonotification = esp3dTFTsettings.readByte(esp3d_auto_notification);
    }
    _started = res;
    if (sendAutoNotificationMsg) {
        sendAutoNotification(ESP3D_NOTIFICATION_ONLINE);
    }
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
    std::string formated_message = message;

    if (formated_message.find('%')!= std::string::npos) {
        formated_message = esp3d_strings::str_replace(formated_message.c_str(), "%ESP_IP%", esp3dNetwork.getLocalIpString());
        formated_message = esp3d_strings::str_replace(formated_message.c_str(), "%ESP_NAME%", esp3dNetwork.getHostName());
    }

    esp3dWsWebUiService.pushNotification(formated_message.c_str());
    if (_started) {
        switch(_notificationType) {
        case esp3d_pushover_notification:
            return sendPushoverMSG(title, formated_message.c_str());
            break;
        case esp3d_telegram_notification:
            return sendTelegramMSG(title, formated_message.c_str());
            break;
        case esp3d_line_notification:
            return sendLineMSG(title, formated_message.c_str());
            break;
        case esp3d_ifttt_notification:
            return sendIFTTTMSG(title, formated_message.c_str());
            break;
        case esp3d_email_notification:
            return sendEmailMSG(title, formated_message.c_str());
            break;
        default:
            break;

        }
    }
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
    if (_autonotification && _started) {
        return sendMSG(ESP3D_NOTIFICATION_TITLE, msg);
    }
    return false;
}

bool Esp3DNotificationsService::sendPushoverMSG(const char * title, const char * message)
{
    return true;
}
bool Esp3DNotificationsService::sendEmailMSG(const char * title, const char * message)
{
    return true;
}
bool Esp3DNotificationsService::sendLineMSG(const char * title, const char * message)
{
    if (_token1.length() == 0 ) {
        esp3d_log_e("Token is missing");
        return false;
    }
    bool res = true;
    esp_http_client_config_t config;
    memset(&config, 0, sizeof(esp_http_client_config_t));
    config.url = _serveraddress.c_str();
    config.port = atoi(_port.c_str());
    config.crt_bundle_attach = esp_crt_bundle_attach,
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        esp3d_log_e("Failed to create http client");
        return false;
    }
    esp3d_log("Client created");
    std::string messageUrl= _serveraddress+":"+_port;
    messageUrl+="/api/notify";
    std::string headerToken = "Bearer " + _token1;
    std::string post_data ="message=";
    post_data+=message;

    esp_http_client_set_header(client, "Host", "notify-api.line.me");
    esp_http_client_set_header(client, "Connection", "close");
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_header(client, "Cache-Control", "no-cache");
    esp_http_client_set_header(client, "User-Agent", "ESP3D");
    esp_http_client_set_header(client, "Authorization", headerToken.c_str());
    esp_http_client_set_url(client,messageUrl.c_str());
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data.c_str(), post_data.length());
    esp3d_log("Try to perform http client: %s", messageUrl.c_str());
    esp_err_t err= esp_http_client_perform(client);
    if (err != ESP_OK) {
        esp3d_log_e("Failed to open HTTP connection: %s", esp_err_to_name(err));
        res = false;
    } else {
        uint code =esp_http_client_get_status_code(client);
        if ( code!= 200) {
            esp3d_log_e("Server response: %d", code);
            res = false;
        }
    }
    esp_http_client_cleanup(client);
    return res;
}
bool Esp3DNotificationsService::sendTelegramMSG(const char * title, const char * message)
{
    if (_token1.length() == 0 || _token2.length()==0) {
        esp3d_log_e("Some token is missing");
        return false;
    }
    bool res = true;
    esp_http_client_config_t config;
    memset(&config, 0, sizeof(esp_http_client_config_t));
    config.url = _serveraddress.c_str();
    config.port = atoi(_port.c_str());
    config.crt_bundle_attach = esp_crt_bundle_attach,
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        esp3d_log_e("Failed to create http client");
        return false;
    }
    esp3d_log("Client created");
    std::string messageUrl= _serveraddress+":"+_port;
    messageUrl+="/bot";
    messageUrl+=_token1;
    messageUrl+="/sendmessage";
    std::string post_data ="chat_id=";
    post_data+=_token2;
    post_data+="&text=";
    post_data+=message;

    esp_http_client_set_header(client, "Host", "api.telegram.org");
    esp_http_client_set_header(client, "Connection", "close");
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_header(client, "Cache-Control", "no-cache");
    esp_http_client_set_header(client, "User-Agent", "ESP3D");
    esp_http_client_set_url(client,messageUrl.c_str());
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data.c_str(), post_data.length());
    esp3d_log("Try to perform http client: %s", messageUrl.c_str());
    esp_err_t err= esp_http_client_perform(client);
// esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        esp3d_log_e("Failed to open HTTP connection: %s", esp_err_to_name(err));
        res = false;
    } else {
        uint code =esp_http_client_get_status_code(client);
        if ( code!= 200) {
            esp3d_log_e("Server response: %d", code);
            res = false;
        }
    }
    esp_http_client_cleanup(client);
    return res;
}
bool Esp3DNotificationsService::sendIFTTTMSG(const char * title, const char * message)
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