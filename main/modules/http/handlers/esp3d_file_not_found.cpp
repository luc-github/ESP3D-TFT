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

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "http/esp3d_http_service.h"

#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_flash.h"

esp_err_t ESP3DHttpService::file_not_found_handler(httpd_req_t *req,
                                                   httpd_err_code_t err) {
  // Send httpd header
  httpd_resp_set_http_hdr(req);
#if ESP3D_AUTHENTICATION_FEATURE
  ESP3DAuthenticationLevel authentication_level = getAuthenticationLevel(req);
  if (authentication_level == ESP3DAuthenticationLevel::guest) {
    // send 401
    return not_authenticated_handler(req);
  }
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
  esp3d_log("Uri: %s Error: %d", req->uri, (int)err);
  std::string uri = req->uri;
  uri = uri.substr(0, uri.find_first_of("?"));

  std::string path;
#if ESP3D_SD_CARD_FEATURE
  if (esp3d_string::startsWith(req->uri, ESP3D_SD_FS_HEADER)) {
    path = uri;
  } else
#endif  // ESP3D_SD_CARD_FEATURE

  {
    path = ESP3D_FLASH_FS_HEADER;
    if (uri[0] == '/') {
      path += &uri[1];  // strip the "first/"
    } else {
      path += uri;
    }
  }

  esp3d_log("Path is %s", path.c_str());

  /*try to stream file*/
  if (ESP_OK == esp3dHttpService.streamFile(path.c_str(), req)) {
    return ESP_OK;
  }

  /*Check custom 404.html*/
  if (ESP_OK == esp3dHttpService.streamFile("/fs/404.html", req)) {
    return ESP_OK;
  }

  /* For any other URI send 404 and close socket */
  httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 - File not found");
  return ESP_FAIL;
}
