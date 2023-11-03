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

#include "authentication/esp3d_authentication_types.h"
#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp_wifi.h"
#include "http/esp3d_http_service.h"
#include "network/esp3d_network.h"

esp_err_t ESP3DHttpService::login_handler(httpd_req_t *req) {
  esp3d_log("Uri: %s", req->uri);
  // Send httpd header
  httpd_resp_set_http_hdr(req);
#if ESP3D_AUTHENTICATION_FEATURE
  std::string tmpstr;
  if (esp3dHttpService.hasArg(req, "DISCONNECT")) {
    tmpstr = esp3dHttpService.getArg(req, "DISCONNECT");
    esp3d_log("DISCONNECT: %s", tmpstr.c_str());
    if (tmpstr == "YES") {
      return not_authenticated_handler(req);
    }
  }
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
  ESP3DAuthenticationLevel level = getAuthenticationLevel(req);
#if ESP3D_AUTHENTICATION_FEATURE
  if (level == ESP3DAuthenticationLevel::guest) {
    // send 401
    return not_authenticated_handler(req);
  }
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
  // send 200
  std::string resp = "{\"status\":\"ok\",\"authentication_lvl\":\"";
  if (level == ESP3DAuthenticationLevel::admin) {
    resp += "admin";
  } else if (level == ESP3DAuthenticationLevel::user) {
    resp += "user";
  } else {
    resp += "guest";
  }
  resp += "\"}";
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, "Authenticated");

  return ESP_OK;
}
