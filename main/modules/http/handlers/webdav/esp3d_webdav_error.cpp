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

esp_err_t ESP3DHttpService::webdav_send_error(httpd_req_t *req, int code,
                                              const char *msg) {
  std::string status = std::to_string(code) + " ";
  switch (code) {
    case 200:
      status += "OK";
      break;
    case 400:
      status += "Bad Request";
      break;
    case 401:
      status += "Unauthorized";
      break;
    case 403:
      status += "Forbidden";
      break;
    case 404:
      status += "Not Found";
      break;
    case 405:
      status += "Method Not Allowed";
    case 406:
      status += "Not Acceptable";
      break;
    case 408:
      status += "Request Timeout";
      break;
    case 409:
      status += "Conflict";
      break;
    case 412:
      status += "Precondition Failed";
      break;
    case 413:
      status += "Request Entity Too Large";
      break;
    case 415:
      status += "Unsupported Media Type";
      break;
    case 423:
      status += "Locked";
      break;
    case 424:
      status += "Failed Dependency";
      break;
    case 501:
      status += "Not Implemented";
      break;
    case 503:
      status += "Service Unavailable";
      break;
    case 507:
      status += "Insufficient Storage";
      break;
    default:
      status += "Internal Server Error";
      break;
  }
  esp3d_log("Uri: %s, status: %s", req->uri, status.c_str());
  return httpd_resp_send_err(req, code, msg);
}
