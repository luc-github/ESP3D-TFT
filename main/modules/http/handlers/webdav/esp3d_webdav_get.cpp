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
#include <sys/utime.h>

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "filesystem/esp3d_globalfs.h"
#include "http/esp3d_http_service.h"
#include "webdav/esp3d_webdav_service.h"

esp_err_t ESP3DHttpService::webdav_get_handler(httpd_req_t *req) {
  esp3d_log("Method: %s", "GET");
  int response_code = 200;
  std::string response_msg = "";
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

  size_t file_size = 0;
  std::string content_type = "";
  std::string last_modified = "";
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
  if (uri == "/") {
    response_code = 404;
    response_msg = "This is not a file";
    esp3d_log_e("Empty uri");
  } else {
    if (globalFs.accessFS(uri.c_str())) {
      struct stat entry_stat;
      if (globalFs.stat(uri.c_str(), &entry_stat) == -1) {
        response_code = 404;
        response_msg = "Failed to stat";
        esp3d_log_e("Failed to stat");
      } else {
        // get last modified time
        last_modified = esp3d_string::getTimeString(entry_stat.st_mtime, true);
        // Add Last-Modified header
        httpd_resp_set_hdr(req, "Last-Modified", last_modified.c_str());
        // is file ?
        if (S_ISREG(entry_stat.st_mode)) {
          // is file
          file_size = entry_stat.st_size;
          content_type = esp3d_string::getContentType(uri.c_str());
          // Add Content-Type header
          httpd_resp_set_type(req, content_type.c_str());
          // Add Content-Length header
          httpd_resp_set_hdr(req, "Content-Length",
                             std::to_string(file_size).c_str());

          // open file
          FILE *fd = globalFs.open(uri.c_str(), "r");
          if (fd) {
            size_t chunksize;
            size_t total_send = 0;
            // send file
            do {
              // Read data block from the file
              chunksize = fread(_chunk, 1, CHUNK_BUFFER_SIZE, fd);
              total_send += chunksize;
              if (chunksize > 0) {
                // Send the HTTP data block
                if (httpd_resp_send_chunk(req, _chunk, chunksize) != ESP_OK) {
                  esp3d_log_e("File sending failed!");
                  chunksize = 0;
                  response_code = 500;
                  response_msg = "Failed to send file";
                }
              }
            } while (chunksize != 0);
            // Close the file
            fclose(fd);
            httpd_resp_send_chunk(req, NULL, 0);
            // Check if all the file has been sent
            if (total_send != file_size) {
              esp3d_log_e("File sending failed: size do not match!");
              response_code = 500;
              response_msg = "File sending failed: size do not match!";
            }
          } else {
            esp3d_log_e("Failed to open file");
            response_code = 500;
            response_msg = "Failed to open file";
          }
        } else {
          // is directory
          esp3d_log_e("This is not a file");
          response_code = 405;
          response_msg = "This is not a file";
        }
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
  if (response_code == 200) return ESP_OK;
  return http_send_response(req, response_code, response_msg.c_str());
}
