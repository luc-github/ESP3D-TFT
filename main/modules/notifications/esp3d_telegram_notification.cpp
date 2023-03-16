/*
  esp3d_notifications
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

#include "esp3d_log.h"
#include "esp3d_notifications_service.h"
#include "esp3d_string.h"
#include "esp_crt_bundle.h"

#define SERVER_URL "https://api.telegram.org"
#define SERVER_PORT 443

bool ESP3DNotificationsService::sendTelegramMSG(const char* title,
                                                const char* message) {
  if (_token1.length() == 0 || _token2.length() == 0) {
    esp3d_log_e("Some token is missing");
    if (_token1.length() == 0) {
      _lastError = ESP3DNotificationError::invalid_token1;
    } else {
      _lastError = ESP3DNotificationError::invalid_token2;
    }
    return false;
  }
  bool res = true;
  esp_http_client_config_t config;
  memset(&config, 0, sizeof(esp_http_client_config_t));
  config.url = SERVER_URL;
  config.port = SERVER_PORT;
  config.crt_bundle_attach = esp_crt_bundle_attach,
  config.transport_type = HTTP_TRANSPORT_OVER_SSL;
  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    esp3d_log_e("Failed to create http client");
    _lastError = ESP3DNotificationError::error;
    return false;
  }
  esp3d_log("Client created");
  std::string messageUrl = SERVER_URL;
  messageUrl += ":";
  messageUrl += std::to_string(SERVER_PORT);
  messageUrl += "/bot";
  messageUrl += _token1;
  messageUrl += "/sendmessage";
  std::string post_data = "chat_id=";
  post_data += _token2;
  post_data += "&text=";
  post_data += message;

  esp_http_client_set_header(client, "Host", "api.telegram.org");
  esp_http_client_set_header(client, "Connection", "close");
  esp_http_client_set_header(client, "Content-Type",
                             "application/x-www-form-urlencoded");
  esp_http_client_set_header(client, "Cache-Control", "no-cache");
  esp_http_client_set_header(client, "User-Agent", "ESP3D");
  esp_http_client_set_url(client, messageUrl.c_str());
  esp_http_client_set_method(client, HTTP_METHOD_POST);
  esp_http_client_set_post_field(client, post_data.c_str(), post_data.length());
  esp3d_log("Try to perform http client: %s", messageUrl.c_str());
  esp_err_t err = esp_http_client_perform(client);
  if (err != ESP_OK) {
    esp3d_log_e("Failed to open HTTP connection: %s", esp_err_to_name(err));
    _lastError = ESP3DNotificationError::error;
    res = false;
  } else {
    uint code = esp_http_client_get_status_code(client);
    if (code != 200) {
      esp3d_log_e("Server response: %d", code);
      if (code == 401) {
        _lastError = ESP3DNotificationError::invalid_token1;
      } else if (code == 400) {
        _lastError = ESP3DNotificationError::invalid_token2;
      } else if (code == 404) {
        _lastError = ESP3DNotificationError::invalid_url;
      } else {
        _lastError = ESP3DNotificationError::invalid_data;
      }
      res = false;
    } else {
      _lastError = ESP3DNotificationError::no_error;
    }
  }
  esp_http_client_cleanup(client);
  return res;
}
