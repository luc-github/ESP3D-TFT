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

#define PROPFIND_RESPONSE_BODY_HEADER_1            \
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" \
  "<D:multistatus xmlns:D=\"DAV:\""
#define PROPFIND_RESPONSE_BODY_HEADER_2 ">\r\n"

#define PROPFIND_RESPONSE_BODY_FOOTER "</D:multistatus>\r\n"

esp_err_t ESP3DHttpService::webdav_propfind_handler(httpd_req_t* req) {
  esp3d_log("Method: %s", "PROPFIND");
  esp3d_log("Uri: %s", req->uri);
  int response_code = 207;
  std::string response_msg = "";
  if (!esp3dHttpService.webdavActive()) {
    int payload_size = _clearPayload(req);
    (void)payload_size;
    response_code = 400;
    response_msg = "Webdav not active";
    esp3d_log_e("Webdav not active");
    return http_send_response(req, response_code, response_msg.c_str());
  }

  std::string response_body = "";
  std::string depth = "0";
  std::string requested_depth = "0";
  esp3d_log("Uri: %s", req->uri);
#if ESP3D_TFT_LOG >= ESP3D_TFT_LOG_LEVEL_DEBUG
  esp3d_log("Headers count: %d\n", showAllHeaders(req));
#endif  // ESP3D_TFT_LOG >= ESP3D_LOG_LEVEL_DEBUG
  std::string uri =
      esp3d_string::urlDecode(&req->uri[strlen(ESP3D_WEBDAV_ROOT) + 1]);
  esp3d_log("Uri: %s", uri.c_str());
  // clear the payload
  int payload_size = _clearPayload(req);
  (void)payload_size;

  // get header Depth
  size_t header_size = httpd_req_get_hdr_value_len(req, "Depth");
  if (header_size > 0) {
    char* header_value = (char*)malloc(header_size + 1);
    if (httpd_req_get_hdr_value_str(req, "Depth", header_value,
                                    header_size + 1) == ESP_OK) {
      depth = header_value;
      requested_depth = header_value;
      esp3d_log("Depth: %s", depth.c_str());
      if (depth != "0" && depth != "1") {
        depth = "1";  // infinity or anything different than 0/1 will be 1
        esp3d_log("Now Depth: %s", depth.c_str());
      }
    }
    free(header_value);
  }
  // sanity check
  if (uri.length() == 0) uri = "/";
  if (uri != "/" && uri[uri.length() - 1] == '/') {
    uri = uri.substr(0, uri.length() - 1);
  }
  if (uri[0] != '/') uri = "/" + uri;
  esp3d_log("Sanity check Uri: %s", uri.c_str());
  // Access file system
  if (globalFs.accessFS(uri.c_str())) {
    struct stat entry_stat;
    // check if file exists
    if (globalFs.stat(uri.c_str(), &entry_stat) == -1) {
      // file does not exist, so no issue to create it
      response_code = 404;
      response_msg = "File not found";
      esp3d_log_e("File not found %s", uri.c_str());
    } else {
      httpd_resp_set_status(req, HTTPD_207);
      httpd_resp_set_type(req, "application/xml; charset=\"utf-8\"");
      // Add Webdav headers
      httpd_resp_set_webdav_hdr(req);

      response_body = PROPFIND_RESPONSE_BODY_HEADER_1;
      if (depth != requested_depth) {
        response_body += " depth=\"" + depth + "\"";
      }
      response_body += PROPFIND_RESPONSE_BODY_HEADER_2;
      httpd_resp_send_chunk(req, response_body.c_str(), response_body.length());
      esp3d_log("%s", response_body.c_str());

      // stat on request first
      response_body = "<D:response xmlns:esp3d=\"DAV:\">\r\n";
      response_body += "<D:href>";
      response_body += "/";
      response_body += ESP3D_WEBDAV_ROOT;
      if (uri != "/") response_body += uri;
      response_body += "</D:href>\r\n";
      response_body +=
          "<D:propstat>\r\n<D:status>HTTP/1.1 200 "
          "OK</D:status>\r\n<D:prop>\r\n";
      response_body += "<esp3d:getlastmodified>";
      response_body += esp3d_string::getTimeString(entry_stat.st_mtime, true);
      response_body += "</esp3d:getlastmodified>\r\n";
      response_body += "<esp3d:creationdate>";
      response_body += esp3d_string::getTimeString(entry_stat.st_ctime, true);
      response_body += "</esp3d:creationdate>\r\n";
      // is dir
      if (S_ISDIR(entry_stat.st_mode)) {
        response_body += "<D:resourcetype><D:collection/></D:resourcetype>\r\n";
        // space entries are only for directory at first level
        // NOTE: letft for future implementation because windows actually does
        // not support it for some reason (tested on windows 11) so it is not
        // really usefull
        /*uint64_t totalSpace = 0;
        uint64_t usedSpace = 0;
        uint64_t freeSpace = 0;
        globalFs.getSpaceInfo(&totalSpace, &usedSpace, &freeSpace, uri.c_str(),
                              true);
        response_body += "<quota-available-bytes>";
        response_body += std::to_string(freeSpace);
        response_body += "</quota-available-bytes>\r\n";
        response_body += "<quota-used-bytes>";
        response_body += std::to_string(usedSpace);
        response_body += "</quota-used-bytes>\r\n";
        response_body += "<fs-total-space>";
        response_body += esp3d_string::formatBytes(totalSpace);
        response_body += "</fs-total-space>\r\n";
        response_body += "<fs-free-space>";
        response_body += esp3d_string::formatBytes(freeSpace);
        response_body += "</fs-free-space>\r\n";
        */
      } else {  // is file
        response_body += "<D:resourcetype/>\r\n";
        response_body += "<esp3d:getcontentlength>" +
                         std::to_string(entry_stat.st_size) +
                         "</esp3d:getcontentlength>\r\n";
      }
      // DisplayName
      response_body += "<esp3d:displayname>";
      if (uri == "/") {
        response_body += "/";
      } else {
        response_body += uri.substr(1);
      }
      response_body += "</esp3d:displayname>\r\n";
      // end tags xml
      response_body += "</D:prop>\r\n</D:propstat>\r\n</D:response>\r\n";
      httpd_resp_send_chunk(req, response_body.c_str(), response_body.length());
      esp3d_log("%s", response_body.c_str());

      // reponse for the first only if dir or file
      if (depth == "1" && S_ISDIR(entry_stat.st_mode)) {
        if (uri[uri.length() - 1] != '/') {
          uri += "/";
        }
        // do direct childs only
        // parse directory direct children
        DIR* dir = globalFs.opendir(uri.c_str());
        std::string currentPath;
        if (dir) {
          struct dirent* entry;
          struct stat entry_stat;
          // parse directory
          while ((entry = globalFs.readdir(dir)) != NULL) {
            currentPath = uri + entry->d_name;

            // stat the entry
            if (globalFs.stat(currentPath.c_str(), &entry_stat) == -1) {
              esp3d_log_e("Failed to stat %s : %s",
                          entry->d_type == DT_DIR ? "DIR" : "FILE",
                          entry->d_name);
              continue;
            }
            // build chunk
            // stat on request first
            response_body = "<D:response xmlns:esp3d=\"DAV:\">\r\n";
            response_body += "<D:href>";
            response_body += "/";
            response_body += ESP3D_WEBDAV_ROOT;
            if (currentPath[0] != '/') response_body += "/";
            response_body += currentPath;
            response_body += "</D:href>\r\n";
            response_body +=
                "<D:propstat>\r\n<D:status>HTTP/1.1 200 "
                "OK</D:status>\r\n<D:prop>\r\n";
            response_body += "<esp3d:getlastmodified>";
            response_body +=
                esp3d_string::getTimeString(entry_stat.st_mtime, true);
            response_body += "</esp3d:getlastmodified>\r\n";
            response_body += "<esp3d:creationdate>";
            response_body +=
                esp3d_string::getTimeString(entry_stat.st_ctime, true);
            response_body += "</esp3d:creationdate>\r\n";
            if (entry->d_type == DT_DIR) {
              // is dir
              response_body +=
                  "<D:resourcetype><D:collection/></D:resourcetype>\r\n";
            } else {
              // is file
              response_body += "<D:resourcetype/>\r\n";
              response_body += "<esp3d:getcontentlength>" +
                               std::to_string(entry_stat.st_size) +
                               "</esp3d:getcontentlength>\r\n";
            }
            // DisplayName
            response_body += "<esp3d:displayname>";
            response_body += entry->d_name;
            response_body += "</esp3d:displayname>\r\n";
            // end tags xml
            response_body += "</D:prop>\r\n</D:propstat>\r\n</D:response>\r\n";
            httpd_resp_send_chunk(req, response_body.c_str(),
                                  response_body.length());
            esp3d_log("\n%s", response_body.c_str());
          }
          globalFs.closedir(dir);
        }
      }
      httpd_resp_send_chunk(req, PROPFIND_RESPONSE_BODY_FOOTER,
                            strlen(PROPFIND_RESPONSE_BODY_FOOTER));
      esp3d_log("%s", PROPFIND_RESPONSE_BODY_FOOTER);
      httpd_resp_send_chunk(req, NULL, 0);
    }

    // release access
    globalFs.releaseFS(uri.c_str());
  } else {
    esp3d_log_e("Failed to access FS: %s", uri.c_str());
    response_code = 503;
    response_msg = "Failed to access FS";
  }

  // send response code to client
  if (response_code == 207) return ESP_OK;
  httpd_resp_set_webdav_hdr(req);
  return http_send_response(req, response_code, response_msg.c_str());
}
