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
#if ESP3D_AUTHENTICATION_FEATURE

#include <stdio.h>

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp_wifi.h"
#include "http/esp3d_http_service.h"

#define HTTPD_401 "401 UNAUTHORIZED" /*!< HTTP Response 401 */
#define HTTPD_401_RESPONSE \
  "{\"status\":\"disconnected\",\"authentication_lvl\":\"guest\"}"
esp_err_t ESP3DHttpService::not_authenticated_handler(httpd_req_t *req) {
  int socketId = httpd_req_to_sockfd(req);
  esp3d_log("Uri: %s, socket: %d", req->uri, socketId);
  esp3dHttpService.onClose(socketId);
  // Send httpd header
  httpd_resp_set_http_hdr(req);
  httpd_resp_set_hdr(req, "Set-Cookie", "ESPSESSIONID=0");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_status(req, HTTPD_401);
  httpd_resp_send(req, HTTPD_401_RESPONSE, strlen(HTTPD_401_RESPONSE));
  return ESP_OK;
}

#endif  // #if ESP3D_AUTHENTICATION_FEATURE