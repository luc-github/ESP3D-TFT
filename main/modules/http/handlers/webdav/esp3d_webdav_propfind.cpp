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
// TODO: Implement method PROPFIND
// Get depth from header
// check for if exists (error 404)
// check if file or directory
// if depth==0 or file just stat the file or directory
// if depth>0 and directory respond with a list of files /directories in the
// directory of one depth
// return the information about the request response
// the response is an xml
// the response is a 207 status code
// Content-Type: application/xml;charset=utf-8
//<? xml version = "1.0" encoding = "utf-8" ?>
//<multistatus xmlns = "DAV:" depth="1">
//  <response>
//    <href>/monrep/</href>
//    <propstat>
//      <prop>
//        <getlastmodified> Tue,10 Jan 2023 09 : 00 : 00 GMT</getlastmodified>
//        <resourcetype>
//          <collection />
//        </ resourcetype>
//      </ prop>
//      <status>HTTP/1.1 200 OK</ status>
//    </propstat>
//  </response>
//
//  <response>
//    <href>/monrep/monsubrep/</ href>
//    <propstat>
//      <prop>
//        <getlastmodified> Tue, 10 Jan 2023 09 : 05 : 00
//        GMT</getlastmodified> <resourcetype>
//          <collection />
//        </resourcetype>
//      </prop>
//      <status>HTTP/1.1 200 OK</status>
//    </propstat>
//  </response>
//
//  <response>
//    <href>/monrep/monfichier.txt</href>
//    <propstat>
//      <prop>
//        <getlastmodified> Tue, 10 Jan 2023 09 : 10 : 00
//        GMT</getlastmodified>
//        <getcontentlength>3000000</getcontentlength>
//        <resourcetype />
//      </prop>
//      <status> HTTP / 1.1 200 OK</status>
//    </propstat>
//  </response>
//</multistatus>

#define PROPFIND_RESPONSE_HEADER \
  "HTTP/1.1 207 Multi-Status\r\nContent-Type: application/xml\r\n"

#define PROPFIND_RESPONSE_BODY_HEADER_1            \
  "<? xml version=\"1.0\" encoding=\"utf-8\" ?>\n" \
  "<multistatus xmlns=\"DAV:\" depth=\""
#define PROPFIND_RESPONSE_BODY_HEADER_2 "\">\n"

#define PROPFIND_RESPONSE_BODY_FOOTER "</multistatus>\n"

esp_err_t ESP3DHttpService::webdav_propfind_handler(httpd_req_t* req) {
  esp3d_log("Uri: %s", req->uri);
  int response_code = 207;
  std::string response_msg = "";
  std::string response_body = "";
  std::string depth = "0";
  esp3d_log_d("Uri: %s", req->uri);
  std::string uri =
      esp3d_string::urlDecode(&req->uri[strlen(ESP3D_WEBDAV_ROOT) + 1]);
  esp3d_log_d("Uri: %s", uri.c_str());
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
      if (depth != "0" && depth != "1") {
        depth = "1";  // infinity or anything different than 0/1 will be 1
      }
    }
    free(header_value);
  }

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
      // Send Chunk
      httpd_resp_send_chunk(req, PROPFIND_RESPONSE_HEADER,
                            strlen(PROPFIND_RESPONSE_HEADER));
      // Add Webdav headers
      httpd_resp_set_webdav_hdr(req, true);
      response_body = PROPFIND_RESPONSE_BODY_HEADER_1 + depth +
                      PROPFIND_RESPONSE_BODY_HEADER_2;
      httpd_resp_send_chunk(req, response_body.c_str(), response_body.length());
      // WebDav Directory name must end with /
      if (S_ISDIR(entry_stat.st_mode)) {
        if (uri[uri.length() - 1] != '/') {
          uri += "/";
        }
      }
      // stat on request first
      response_body = "<response>\n";
      response_body += "<href>";
      response_body += uri;
      response_body += "</href>\n";
      response_body += "<propstat>\n<prop>\n";
      response_body += "<getlastmodified>";
      response_body += esp3d_string::getTimeString(entry_stat.st_mtime);
      response_body += "</getlastmodified>\n";
      // is dir
      if (S_ISDIR(entry_stat.st_mode)) {
        response_body += "<resourcetype><collection /></resourcetype>\n";
        // space entries are only for directory at first level
        uint64_t totalSpace = 0;
        uint64_t usedSpace = 0;
        uint64_t freeSpace = 0;
        globalFs.getSpaceInfo(&totalSpace, &usedSpace, &freeSpace, uri.c_str(),
                              true);
        response_body += "<quota-available-bytes>";
        response_body += std::to_string(freeSpace);
        response_body += "</quota-available-bytes>\n";
        response_body += "<quota-used-bytes>";
        response_body += std::to_string(usedSpace);
        response_body += "</quota-used-bytes>\n";
        response_body += "<fs-total-space>";
        response_body += esp3d_string::formatBytes(totalSpace);
        response_body += "</fs-total-space>\n";
        response_body += "<fs-free-space>";
        response_body += esp3d_string::formatBytes(freeSpace);
        response_body += "</fs-free-space>\n";
      } else {  // is file
        response_body += "<resourcetype/>\n";
        response_body += "<getcontentlength>" +
                         std::to_string(entry_stat.st_size) +
                         "</getcontentlength>\n";
      }

      // end tags xml
      response_body += "</prop>\n<status>HTTP/1.1 200 OK</status>\n ";
      response_body += "</propstat>\n</response>\n";
      httpd_resp_send_chunk(req, response_body.c_str(), response_body.length());

      // reponse for the first only if dir or file
      if (depth == "1" && S_ISDIR(entry_stat.st_mode)) {
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
            // in webdav we need to add / at the end of directory nam/path
            if (entry->d_type == DT_DIR) {
              currentPath += "/";
            }

            // stat on request first
            response_body = "<response>\n";
            response_body += "<href>";
            response_body += currentPath;
            response_body += "</href>\n";
            response_body += "<propstat>\n<prop>\n";
            response_body += "<getlastmodified>";
            response_body += esp3d_string::getTimeString(entry_stat.st_mtime);
            response_body += "</getlastmodified>\n";

            if (entry->d_type == DT_DIR) {
              // is dir
              response_body += "<resourcetype><collection /></resourcetype>\n";
            } else {
              // is file
              response_body += "<resourcetype/>\n";
              response_body += "<getcontentlength>" +
                               std::to_string(entry_stat.st_size) +
                               "</getcontentlength>\n";
            }
            // end tags xml
            response_body +=
                "</prop>\n<status>HTTP/1.1 200 "
                "OK</status>\n</propstat>\n</response>\n";
            httpd_resp_send_chunk(req, response_body.c_str(),
                                  response_body.length());
          }
          globalFs.closedir(dir);
        }
      }
      httpd_resp_send_chunk(req, PROPFIND_RESPONSE_BODY_FOOTER,
                            strlen(PROPFIND_RESPONSE_BODY_FOOTER));
      httpd_resp_send_chunk(req, NULL, 0);
    }

    // release access
    globalFs.releaseFS(uri.c_str());
  } else {
    esp3d_log_e("Failed to access FS");
    response_code = 503;
    response_msg = "Failed to access FS";
  }

  // send response code to client
  if (response_code == 207) return ESP_OK;
  httpd_resp_set_webdav_hdr(req);
  return http_send_response(req, response_code, response_msg.c_str());
}
