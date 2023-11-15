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

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "http/esp3d_http_service.h"
#include "webdav/esp3d_webdav_service.h"

esp_err_t ESP3DHttpService::webdav_options_handler(httpd_req_t *req) {
  int response_code = 200;
  std::string response_msg = "";
  esp3d_log("Method: %s", "OPTIONS");
  esp3d_log("Uri: %s", req->uri);
  if (!esp3dHttpService.webdavActive()) {
    int payload_size = _clearPayload(req);
    (void)payload_size;
    response_code = 400;
    response_msg = "Webdav not active";
    esp3d_log_e("Webdav not active");
    return http_send_response(req, response_code, response_msg.c_str());
  }
  std::string uri =
      esp3d_string::urlDecode(&req->uri[strlen(ESP3D_WEBDAV_ROOT) + 1]);
  esp3d_log("Uri: %s", uri.c_str());
#if ESP3D_TFT_LOG >= ESP3D_TFT_LOG_LEVEL_DEBUG
  esp3d_log("Headers count: %d\n", showAllHeaders(req));
#endif  // ESP3D_TFT_LOG >= ESP3D_LOG_LEVEL_
  int payload_size = _clearPayload(req);
  (void)payload_size;
  esp3d_log("Payload size: %d", payload_size);
  // Add Webdav headers
  httpd_resp_set_webdav_hdr(req);
  // response_code is 200
  return http_send_response(req, response_code, "");
  ;
}
