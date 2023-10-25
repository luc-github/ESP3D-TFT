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

esp_err_t ESP3DHttpService::webdav_delete_handler(httpd_req_t *req) {
  int response_code = 204;
  std::string response_msg = "Success";
  esp3d_log("Uri: %s", req->uri);
  std::string uri =
      esp3d_string::urlDecode(&req->uri[strlen(ESP3D_WEBDAV_ROOT) + 1]);
  esp3d_log("Uri: %s", uri.c_str());

  int payload_size = _clearPayload(req);
  (void)payload_size;
  esp3d_log("Payload size: %d", payload_size);
  // Add Webdav headers
  httpd_resp_set_webdav_hdr(req);
  // sanity check
  if (uri.length() == 0) uri = "/";
  if (uri == "/" ||
      strncmp(uri.c_str(), ESP3D_FLASH_FS_HEADER,
              strlen(ESP3D_FLASH_FS_HEADER) - 1) == 0 ||
      strncmp(uri.c_str(), ESP3D_SD_FS_HEADER,
              strlen(ESP3D_SD_FS_HEADER) - 1) == 0) {
    response_code = 400;
    response_msg = "Not allowed";
    esp3d_log_e("Empty uri");
  } else {
    // Check can access (error code 503)
    if (globalFs.accessFS(uri.c_str())) {
      struct stat entry_stat;
      if (globalFs.stat(uri.c_str(), &entry_stat) == -1) {
        response_code = 404;
        response_msg = "Failed to stat";
      } else {
        if (S_ISDIR(entry_stat.st_mode)) {
          // is directory
          if (!globalFs.rmdir(uri.c_str())) {
            esp3d_log_e("Failed to delete dir");
            response_code = 500;
            response_msg = "Failed to delete dir";
          }
        } else {
          // is file
          if (!globalFs.remove(uri.c_str())) {
            esp3d_log_e("Failed to delete file");
            response_code = 500;
            response_msg = "Failed to delete file";
          }
        }
      }
      // release access
      globalFs.releaseFS(uri.c_str());
    } else {
      esp3d_log_e("Failed to access FS");
      response_code = 503;
      response_msg = "Failed to access FS";
    }
  }
  // send response code to client
  return http_send_response(req, response_code, response_msg.c_str());
}
