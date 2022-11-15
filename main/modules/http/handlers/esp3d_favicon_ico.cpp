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


esp_err_t Esp3DHttpService::favicon_ico_handler(httpd_req_t *req)
{
    esp3d_log("Uri: %s", req->uri);
    esp_err_t err = esp3dHttpService.streamFile("/fs/favicon.ico", req);
    if ( err == ESP_ERR_NOT_FOUND) {
        extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_gz_start");
        extern const unsigned char favicon_ico_end[]   asm("_binary_favicon_ico_gz_end");
        const size_t favicon_ico_size = (favicon_ico_end - favicon_ico_start);
        httpd_resp_set_type(req, "image/x-icon");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        err = httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);
    } else if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
        esp3d_log_e("Raise exception 500 INTERNAL SERVER ERROR");
    }
    //success or fail ?
    return err== ESP_OK?ESP_OK:ESP_FAIL;
}
