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
#if ESP3D_TIMESTAMP_FEATURE
#include "time/esp3d_time_service.h"
#else
#include <time.h>

#endif  // ESP3D_TIMESTAMP_FEATURE
#include "tasks_def.h"

#define CHUNK_PUT_BUFFER_SIZE STREAM_CHUNK_SIZE * 4

esp_err_t ESP3DHttpService::webdav_put_handler(httpd_req_t* req) {
  esp3d_log("Method: %s", "PUT");
  esp3d_log("Uri: %s", req->uri);
  int response_code = 201;
  std::string response_msg = "";
  if (!esp3dHttpService.webdavActive()) {
    int payload_size = _clearPayload(req);
    (void)payload_size;
    response_code = 400;
    response_msg = "Webdav not active";
    esp3d_log_e("Webdav not active");
    return http_send_response(req, response_code, response_msg.c_str());
  }

  size_t file_size = 0;
  size_t total_read = 0;
  std::string last_modified = "";
  esp3d_log("Uri: %s", req->uri);
#if ESP3D_TFT_LOG >= ESP3D_TFT_LOG_LEVEL_DEBUG
  esp3d_log("Headers count: %d\n", showAllHeaders(req));
#endif  // ESP3D_TFT_LOG >= ESP3D_LOG_LEVEL_DEBUG
  std::string uri =
      esp3d_string::urlDecode(&req->uri[strlen(ESP3D_WEBDAV_ROOT) + 1]);
  esp3d_log("Uri: %s", uri.c_str());
  bool overwrite = true;

  // get content length
  size_t header_size = httpd_req_get_hdr_value_len(req, "Content-Length");
  if (header_size > 0) {
    char* header_value = (char*)malloc(header_size + 1);
    if (httpd_req_get_hdr_value_str(req, "Content-Length", header_value,
                                    header_size + 1) == ESP_OK) {
      file_size = atoi(header_value);
    }
    esp3d_log("Content-Length: %d", file_size);
    free(header_value);
  }

  // Add Webdav headers
  httpd_resp_set_webdav_hdr(req);

  // sanity check
  if (uri.length() == 0) uri = "/";
  if (uri == "/") {
    response_code = 400;
    response_msg = "Not allowed";
    esp3d_log_e("Empty uri");
  } else {
    // Access file system
    if (globalFs.accessFS(uri.c_str())) {
      struct stat entry_stat;
      // check if file exists
      if (globalFs.stat(uri.c_str(), &entry_stat) == -1) {
        // file does not exist, so no issue to create it
        overwrite = true;
      } else {                              // file exists
        if (S_ISDIR(entry_stat.st_mode)) {  // it is a directory
          // is directory
          response_code = 412;
          response_msg = "Directory has same name as file";
          overwrite = false;
          esp3d_log_e("Directory has same name as file");
        } else {             // it is a file
          overwrite = true;  // overwrite by default
          response_code = 204;
        }
      }
      if (overwrite) {
        time_t now;
        time(&now);
        last_modified = esp3d_string::getTimeString(now, true);

        httpd_resp_set_hdr(req, "Last-Modified", last_modified.c_str());
        // check free space
        uint64_t totalBytes;
        uint64_t usedBytes;
        uint64_t freeBytes;
        if (globalFs.getSpaceInfo(&totalBytes, &usedBytes, &freeBytes,
                                  uri.c_str(), true)) {
          if (freeBytes < file_size) {
            response_code = 507;
            response_msg = "Not enough space";
            esp3d_log_e("Not enough space");
          } else {
            FILE* fd = globalFs.open(uri.c_str(), "w");
            if (fd) {
              bool hasError = false;
              if (file_size > 0) {
                char* packetWrite = nullptr;
                size_t remaining = file_size;
                size_t received = 0;
                packetWrite = (char*)malloc(CHUNK_PUT_BUFFER_SIZE + 1);
                if (packetWrite) {
                  while (remaining > 0 && !hasError) {
                    if ((received = httpd_req_recv(
                             req, packetWrite, CHUNK_PUT_BUFFER_SIZE)) <= 0) {
                      esp3d_log_e("Connection lost");
                      hasError = true;
                    }
                    if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                      esp3d_log_e("Time out");
                      continue;
                    }
                    if (received == HTTPD_SOCK_ERR_INVALID ||
                        received == HTTPD_SOCK_ERR_FAIL) {
                      esp3d_log_e("Error connection");
                      hasError = true;
                    }
                    // decrease received bytes from
                    // remaining bytes amount
                    remaining -= received;
                    // write packet to file
                    if (received > 0) {
                      packetWrite[received] = 0;
                      if (fwrite(packetWrite, 1, received, fd) != received) {
                        esp3d_log_e("Error writing file");
                        hasError = true;
                      }
                      total_read += received;
                    }
                  }
                  free(packetWrite);
                  if (hasError) {
                    response_code = 500;
                    response_msg = "Error writing file";
                    esp3d_log_e("Error writing file");
                  }
                } else {
                  esp3d_log_e("Failed to allocate memory");
                  response_code = 500;
                  response_msg = "Failed to allocate memory";
                }
              }
              // Close the file
              fclose(fd);
              // Check if all the file has been sent
              if (hasError && total_read != file_size) {
                esp3d_log_e(
                    "File receiving failed: size do not "
                    "match!");
                response_code = 500;
                response_msg =
                    "File receiving failed: size do not "
                    "match!";
              }
            } else {
              esp3d_log_e("Failed to open file");
              response_code = 500;
              response_msg = "Failed to open file";
            }
          }
        } else {
          esp3d_log_e("Failed to get space info");
          response_code = 500;
          response_msg = "Failed to get space info";
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
  return http_send_response(req, response_code, "");
}
