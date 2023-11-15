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

#define CHUNK_COPY_BUFFER_SIZE STREAM_CHUNK_SIZE * 4

esp_err_t ESP3DHttpService::webdav_copy_handler(httpd_req_t* req) {
  int response_code = 201;
  std::string response_msg = "Success";
  std::string destination;
  bool destination_exists = false;
  size_t file_size = 0;

  bool overwrite = false;
  esp3d_log("Method: %s", "COPY");
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
      size_t pos = dest.find(ESP3D_WEBDAV_ROOT);
      if (pos != std::string::npos) {
        uri = dest.substr(pos + strlen(ESP3D_WEBDAV_ROOT));
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
      // check if source exists
      if (globalFs.stat(uri.c_str(), &entry_stat) == -1) {
        response_code = 404;
        esp3d_log_e("Source does not exist");
      } else {  // source exists
        // check if source is a file
        if (S_ISDIR(entry_stat.st_mode)) {
          response_code = 413;
          response_msg = "Source is a directory";
          esp3d_log_e("Source is a directory");
        } else {
          // do we have space ?
          file_size = entry_stat.st_size;
          // check free space
          uint64_t totalBytes;
          uint64_t usedBytes;
          uint64_t freeBytes;
          if (globalFs.getSpaceInfo(&totalBytes, &usedBytes, &freeBytes,
                                    uri.c_str(), true)) {
            if (freeBytes > file_size) {
              // check if destination exists
              if (globalFs.stat(destination.c_str(), &entry_stat) == -1) {
                // destination does not exist
                overwrite = true;
              } else {
                // destination exists
                destination_exists = true;
                if (S_ISDIR(entry_stat.st_mode)) {
                  // destination is a directory
                  response_code = 412;
                  response_msg = "Destination is a directory";
                  overwrite = false;
                  esp3d_log_e("Destination is a directory");
                } else {
                  if (!overwrite) {
                    response_code = 412;
                    response_msg = "Destination exists";
                    esp3d_log_e("Destination exists");
                  }
                }
              }
              // can we copy ?
              if (overwrite) {
                // open source file
                FILE* source_file = globalFs.open(uri.c_str(), "r");
                // open destination file
                FILE* destination_file =
                    globalFs.open(destination.c_str(), "w");
                if (source_file && destination_file) {
                  // let's copy
                  char* packetWrite = (char*)malloc(CHUNK_COPY_BUFFER_SIZE + 1);
                  if (packetWrite) {
                    size_t remaining = file_size;
                    size_t received = 0;
                    while (remaining > 0) {
                      if ((received =
                               fread(packetWrite, 1, CHUNK_COPY_BUFFER_SIZE,
                                     source_file)) <= 0) {
                        esp3d_log_e("Failed to read source file");
                        response_code = 500;
                        response_msg = "Failed to read source file";
                        break;
                      }
                      if (fwrite(packetWrite, 1, received, destination_file) !=
                          received) {
                        esp3d_log_e("Failed to write destination file");
                        response_code = 500;
                        response_msg = "Failed to write destination file";
                        break;
                      }
                      remaining -= received;
                    }

                    if (remaining == 0) {
                      // success for the copy
                      // adjust response code if destination exists
                      if (destination_exists) {
                        response_code = 204;
                      }
                    }
                    free(packetWrite);
                  } else {
                    esp3d_log_e("Failed to allocate memory");
                    response_code = 500;
                    response_msg = "Failed to allocate memory";
                  }
                } else {
                  response_code = 500;
                  response_msg = "Failed to open file";
                  esp3d_log_e("Failed to open file");
                }
                if (source_file) globalFs.close(source_file, uri.c_str());
                if (destination_file)
                  globalFs.close(destination_file, destination.c_str());
              }

            } else {
              response_code = 507;
              response_msg = "Insufficient storage";
              esp3d_log_e("Insufficient storage");
            }
          } else {
            esp3d_log_e("Failed to get space info");
            response_code = 500;
            response_msg = "Failed to get space info";
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
