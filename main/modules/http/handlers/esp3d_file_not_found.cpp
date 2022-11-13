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


#include "http/esp3d_http_service.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp3d_commands.h"
#include "network/esp3d_network.h"


esp_err_t Esp3DHttpService::file_not_found_handler(httpd_req_t *req, httpd_err_code_t err)
{
    esp3d_log("Uri: %s", req->uri);
    /* if (strcmp("/hello", req->uri) == 0) {
     httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
     // Return ESP_OK to keep underlying socket open
     return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
     httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
     // Return ESP_FAIL to close underlying socket
     return ESP_FAIL;
    }*/
    /* For any other URI send 404 and close socket */

    //TODO: handle custom 404 file
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 - File not found");
    return ESP_FAIL;
}
