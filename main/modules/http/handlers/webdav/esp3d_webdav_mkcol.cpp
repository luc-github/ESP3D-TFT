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
#include "filesystem/esp3d_globalfs.h"
#include "http/esp3d_http_service.h"
#include "webdav/esp3d_webdav_service.h"

esp_err_t ESP3DHttpService::webdav_mkcol_handler(httpd_req_t* req) {
  int response_code = 201;
  std::string response_msg = "Success";
  esp3d_log("Method: %s", "MKCOL");
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
  // get header Destination
  size_t header_size = httpd_req_get_hdr_value_len(req, "Destination");
  if (header_size > 0) {
    char* header_value = (char*)malloc(header_size + 1);
    if (httpd_req_get_hdr_value_str(req, "Destination", header_value,
                                    header_size + 1) == ESP_OK) {
      // replace uri by destination
      std::string dest = header_value;
      esp3d_log("Destination Header: %s", dest.c_str());
      size_t pos = dest.find(ESP3D_WEBDAV_ROOT);
      if (pos != std::string::npos) {
        uri = dest.substr(pos + strlen(ESP3D_WEBDAV_ROOT));
        esp3d_log("Destination Uri: %s", uri.c_str());
      }
    }
    free(header_value);
  }
  // clear payload from request if any
  int payload_size = _clearPayload(req);
  (void)payload_size;
  esp3d_log("Payload size: %d", payload_size);
  // Add Webdav headers
  httpd_resp_set_webdav_hdr(req);
  // sanity check
  if (uri.length() == 0) uri = "/";

  if (uri == "/" || uri == ESP3D_FLASH_FS_HEADER || uri == ESP3D_SD_FS_HEADER ||
      std::string(uri + "/") == ESP3D_FLASH_FS_HEADER ||
      std::string(uri + "/") == ESP3D_SD_FS_HEADER) {
    response_code = 400;
    response_msg = "Not allowed";
    esp3d_log_e("Empty uri");
  } else {
    // Check can access (error code 503)
    if (globalFs.accessFS(uri.c_str())) {
      struct stat entry_stat;
      if (globalFs.stat(uri.c_str(), &entry_stat) == -1) {
        // does not exist
        if (!globalFs.mkdir(uri.c_str())) {
          esp3d_log_e("Failed to create dir");
          response_code = 500;
          response_msg = "Failed to create dir";
        }
      } else {  // already exists
        response_code = 409;
        response_msg = "Already exists";
      }
      // release access
      globalFs.releaseFS(uri.c_str());
    } else {
      esp3d_log_e("Failed to access FS: %s", uri.c_str());
      response_code = 503;
      response_msg = "Failed to access FS";
    }
  }
  // send response code to client
  return http_send_response(req, response_code, response_msg.c_str());
}
