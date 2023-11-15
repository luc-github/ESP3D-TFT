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

esp_err_t ESP3DHttpService::webdav_move_handler(httpd_req_t* req) {
  int response_code = 201;
  std::string response_msg = "Success";
  std::string destination;
  bool source_is_file = false;
  bool destination_is_file = false;
  bool destination_exists = false;

  bool overwrite = false;
  esp3d_log("Method: %s", "MOVE");
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
  size_t header_size = 0;
  esp3d_log("Uri: %s", uri.c_str());

  // get header Destination
  header_size = httpd_req_get_hdr_value_len(req, "Destination");
  if (header_size > 0) {
    char* header_value = (char*)malloc(header_size + 1);
    if (httpd_req_get_hdr_value_str(req, "Destination", header_value,
                                    header_size + 1) == ESP_OK) {
      // replace uri by destination
      destination = header_value;
      size_t pos = destination.find(ESP3D_WEBDAV_ROOT);
      if (pos != std::string::npos) {
        destination = destination.substr(pos + strlen(ESP3D_WEBDAV_ROOT));
      }
    }
    free(header_value);
  }
  // get header overwrite
  header_size = httpd_req_get_hdr_value_len(req, "Overwrite");
  if (header_size > 0) {
    char* header_value = (char*)malloc(header_size + 1);
    if (httpd_req_get_hdr_value_str(req, "Overwrite", header_value,
                                    header_size + 1) == ESP_OK) {
      // check value of overwrite
      // any other value than T is considered as false
      if (strcmp(header_value, "T") == 0) overwrite = true;
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
        response_code = 404;
        esp3d_log_e("Source does not exist");
      } else {
        if (S_ISREG(entry_stat.st_mode)) {
          source_is_file = true;
        }
        // check if destination exists
        if (globalFs.stat(destination.c_str(), &entry_stat) == -1) {
          overwrite = true;
          destination_is_file = source_is_file;
        } else {
          destination_exists = true;
          if (S_ISREG(entry_stat.st_mode)) {
            destination_is_file = true;
          }
        }
        // only copy /fs to /fs and /sd to /sd
        if (globalFs.getFSType(uri.c_str()) !=
            globalFs.getFSType(destination.c_str())) {
          overwrite = false;
          response_code = 400;
          response_msg = "Not allowed";
          esp3d_log_e("Not allowed");
        }
        // check if overwrite is allowed, overwrite means Ok to move
        if (overwrite) {
          if (source_is_file == destination_is_file) {
            // delete destination if exists and is directory
            if (destination_exists && !destination_is_file) {
              if (!globalFs.rmdir(destination.c_str())) {
                esp3d_log_e("Failed to delete dir");
                response_code = 500;
                response_msg = "Failed to delete dir";
              }
            }
            // delete destination if exists and is file
            if (destination_exists && destination_is_file) {
              if (!globalFs.remove(destination.c_str())) {
                esp3d_log_e("Failed to delete file");
                response_code = 500;
                response_msg = "Failed to delete file";
              }
            }
            // rename source to destination
            if (!globalFs.rename(uri.c_str(), destination.c_str())) {
              esp3d_log_e("Failed to rename");
              response_code = 500;
              response_msg = "Failed to rename";
            } else {
              // success for the move
              // adjust response code if destination exists
              if (destination_exists) {
                response_code = 204;
              }
            }
          } else {
            response_code = 412;
            esp3d_log_e("Overwrite not allowed");
          }
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
  return http_send_response(req, response_code, response_msg.c_str());
}
