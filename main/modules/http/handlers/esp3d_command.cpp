/*
  esp3d_http_service
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

#include <stdio.h>

#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp_wifi.h"
#include "http/esp3d_http_service.h"
#include "network/esp3d_network.h"

esp_err_t ESP3DHttpService::command_handler(httpd_req_t *req) {
  // TODO: check authentication level
  ESP3DAuthenticationLevel authentication_level = getAuthenticationLevel(req);
  // Send httpd header
  httpd_resp_set_http_hdr(req);
#if ESP3D_AUTHENTICATION_FEATURE
  if (authentication_level == ESP3DAuthenticationLevel::guest) {
    // send 401
    return not_authenticated_handler(req);
  }
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
  esp3d_log("Uri: %s", req->uri);
  char *buf;
  size_t buf_len;
  char cmd[255 + 1] = {0};
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char *)malloc(buf_len);
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      esp3d_log("query string: %s", buf);
      if (httpd_query_key_value(buf, "cmd", cmd, 255) == ESP_OK) {
        strcpy(cmd, esp3d_string::urlDecode(cmd));
        esp3d_log("command is: %s", cmd);
      } else if (httpd_query_key_value(buf, "PING", cmd, 255) == ESP_OK) {
        if (strcmp(cmd, "PING") == 0) {
          esp3d_log("command is a PING");
          // we do not care the value a command was sent and reset is already
          // done
          free(buf);
          // Just do nothing
          return ESP_OK;
        }

      } else {
        esp3d_log_e("Invalid param");
      }
    }
    free(buf);
  }
  if (strlen(cmd) > 0) {
    ESP3DRequest requestId;
    if (esp3dCommands.is_esp_command((uint8_t *)cmd, strlen(cmd))) {
      requestId.http_request = req;
      ESP3DMessage *newMsgPtr = ESP3DClient::newMsg(
          ESP3DClientType::webui, ESP3DClientType::command,
          (const uint8_t *)cmd, strlen(cmd), authentication_level);
      if (newMsgPtr) {
        newMsgPtr->request_id.http_request = req;
        esp3dCommands.process(newMsgPtr);
        return ESP_OK;
      } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Message creation failed");
      }
    } else {
      httpd_resp_sendstr(req, "ESP3D says: command forwarded");
      requestId.id = 0;
      // check format is correct
      if (!esp3d_string::endsWith(cmd, "\n")) {
        if (!esp3dCommands.formatCommand(cmd, 255)) {
          esp3d_log_e("Error command is too long");
          return ESP_FAIL;
        }
      }
      esp3dCommands.dispatch(cmd, ESP3DClientType::stream, requestId,
                             ESP3DMessageType::unique, ESP3DClientType::webui,
                             authentication_level);

      return ESP_OK;
    }

  } else {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request");
  }
  return ESP_FAIL;
}
