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

esp_err_t ESP3DHttpService::http_send_response(httpd_req_t *req, int code,
                                               const char *msg) {
  std::string status = std::to_string(code) + " ";
  switch (code) {
    case 200:
      status = HTTPD_200;
      break;
    case 201:
      status += "Created";
      break;
    case 202:
      status += "Accepted";
      break;
    case 204:
      status = HTTPD_204;
      break;
    case 207:
      status = HTTPD_207;
      break;
    case 400:
      status = HTTPD_400;
      break;
    case 401:
      status += "Unauthorized";
      break;
    case 403:
      status += "Forbidden";
      break;
    case 404:
      status = HTTPD_404;
      break;
    case 405:
      status += "Method Not Allowed";
      break;
    case 406:
      status += "Not Acceptable";
      break;
    case 408:
      status = HTTPD_408;
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
    case 500:
      status = HTTPD_500;
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
      status += "";
      esp3d_log_e("Unknown status code: %d", code);
      break;
  }
  httpd_resp_set_status(req, status.c_str());
  esp3d_log("%s", req->uri);
  esp3d_log("Status: %s", status.c_str());
  esp3d_log("%s", msg);
  return httpd_resp_send(req, msg, strlen(msg));
}
