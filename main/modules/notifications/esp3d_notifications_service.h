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
#include "esp_http_client.h"
#include "mbedtls/net_sockets.h"
#include "notifications/customizations.h"

#ifdef __cplusplus
extern "C" {
#endif

enum class ESP3DNotificationError : uint8_t {
  no_error,
  empty_message,
  invalid_message,
  invalid_data,
  invalid_url,
  invalid_token1,
  invalid_token2,
  error,
};

enum class ESP3DNotificationType : uint8_t {
  none,
  pushover,
  email,
  line,
  telegram,
  ifttt
};

class ESP3DNotificationsService final {
 public:
  ESP3DNotificationsService();
  ~ESP3DNotificationsService();
  bool begin(bool sendAutoNotificationMsg = false);
  void handle();
  void end();
  bool sendMSG(const char *title, const char *message);
  bool sendPushoverMSG(const char *title, const char *message);
  bool sendEmailMSG(const char *title, const char *message);
  bool sendLineMSG(const char *title, const char *message);
  bool sendTelegramMSG(const char *title, const char *message);
  bool sendIFTTTMSG(const char *title, const char *message);
  const char *getTypeString();
  ESP3DNotificationType getType() { return _notificationType; }
  bool started() { return _started; }
  bool isAutonotification() { return _autonotification; };
  void setAutonotification(bool value) { _autonotification = value; };
  bool sendAutoNotification(const char *msg);
  ESP3DNotificationError getLastError() { return _lastError; }
  int perform_tls_handshake(mbedtls_ssl_context *ssl);
  int write_ssl_and_get_response(mbedtls_ssl_context *ssl, unsigned char *buf,
                                 size_t len);
  int write_tls_and_get_response(mbedtls_net_context *sock_fd,
                                 unsigned char *buf, size_t len);

 private:
  bool _started;
  bool _autonotification;
  ESP3DNotificationType _notificationType;
  std::string _token1;
  std::string _token2;
  std::string _settings;
  std::string _serveraddress;
  std::string _port;
  std::string _method;
  ESP3DNotificationError _lastError;
  bool getEmailInformationsFromSettings();
};

extern ESP3DNotificationsService esp3dNotificationsService;
#ifdef __cplusplus
}  // extern "C"
#endif