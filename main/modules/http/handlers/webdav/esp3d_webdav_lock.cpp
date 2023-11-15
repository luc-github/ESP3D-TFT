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
#include "filesystem/esp3d_globalfs.h"
#include "http/esp3d_http_service.h"
#include "webdav/esp3d_webdav_service.h"

#define LOCK_RESPONSE_PART_1                   \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
  "<D:prop xmlns:D=\"DAV:\">"                  \
  "<D:lockdiscovery>"                          \
  "<D:activelock>"                             \
  "<D:locktoken>"                              \
  "<D:href>"

#define LOCK_RESPONSE_PART_2 \
  "</D:href>"                \
  "</D:locktoken>"           \
  "</D:activelock>"          \
  "</D:lockdiscovery>"       \
  "</D:prop>"

esp_err_t ESP3DHttpService::webdav_lock_handler(httpd_req_t *req) {
  int response_code = 200;
  std::string response_msg = "Success";
  esp3d_log("Method: %s", "LOCK");
  esp3d_log("Uri: %s", req->uri);
  if (!esp3dHttpService.webdavActive()) {
    int payload_size = _clearPayload(req);
    (void)payload_size;
    response_code = 400;
    response_msg = "Webdav not active";
    esp3d_log_e("Webdav not active");
    return http_send_response(req, response_code, response_msg.c_str());
  }
#if ESP3D_TFT_LOG >= ESP3D_TFT_LOG_LEVEL_DEBUG
  esp3d_log("Headers count: %d\n", showAllHeaders(req));
#endif  // ESP3D_TFT_LOG >= ESP3D_LOG_LEVEL_
  std::string uri =
      esp3d_string::urlDecode(&req->uri[strlen(ESP3D_WEBDAV_ROOT) + 1]);
  esp3d_log("Uri: %s", uri.c_str());

  int payload_size = _clearPayload(req);
  (void)payload_size;
  esp3d_log("Payload size: %d", payload_size);

  // sanity check
  if (uri.length() == 0) uri = "/";
  if (uri == "/" || uri == ESP3D_FLASH_FS_HEADER || uri == ESP3D_SD_FS_HEADER ||
      std::string(uri + "/") == ESP3D_FLASH_FS_HEADER ||
      std::string(uri + "/") == ESP3D_SD_FS_HEADER) {
    response_code = 400;
    response_msg = "Not allowed";
    esp3d_log_e("wrong uri");
  } else {
    // Check can access (error code 503)
    if (globalFs.accessFS(uri.c_str())) {
      struct stat entry_stat;
      if (globalFs.stat(uri.c_str(), &entry_stat) == -1) {
        response_code = 404;
        response_msg = "Failed to stat";
      } else {
        // generate lock token
        std::string lock_token = "opaquelocktoken:";
        lock_token += esp3d_string::generateUUID(uri.c_str());
        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_set_type(req, "application/xml; charset=\"utf-8\"");
        httpd_resp_set_webdav_hdr(req);
        // add header
        httpd_resp_set_hdr(req, "Lock-Token", lock_token.c_str());
        response_msg = LOCK_RESPONSE_PART_1;
        response_msg += lock_token;
        response_msg += LOCK_RESPONSE_PART_2;
        httpd_resp_send(req, response_msg.c_str(), response_msg.length());
      }
      // release access
      globalFs.releaseFS(uri.c_str());
    } else {
      esp3d_log_e("Failed to access FS: %s", uri.c_str());
      response_code = 503;
      response_msg = "Failed to access FS";
    }
  }
  if (response_code == 200) return ESP_OK;
  httpd_resp_set_webdav_hdr(req);
  // send response code to client
  return http_send_response(req, response_code, response_msg.c_str());
}
