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


esp_err_t Esp3DHttpService::root_get_handler(httpd_req_t *req)
{
    //httpd_resp_send(req, NULL, 0);
    esp3d_log("Uri: %s", req->uri);
    char*  buf;
    size_t buf_len;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char value[255]= {0};
            esp3d_log("query string: %s", buf);
            if (httpd_query_key_value(buf, "forcefallback", value, 254)==ESP_OK) {
                esp3d_log("forcefallback value: %s", esp3d_strings::urlDecode(value));
            } else {
                esp3d_log("Invalid param");
            }
            if (httpd_query_key_value(buf, "cmd", value, 254)==ESP_OK) {
                esp3d_log("cmd value: %s", esp3d_strings::urlDecode(value));
            } else {
                esp3d_log("Invalid param");
            }
        }
        free(buf);
    }
    extern const unsigned char index_html_gz_start[] asm("_binary_index_html_gz_start");
    extern const unsigned char index_html_gz_end[]   asm("_binary_index_html_gz_end");
    const size_t index_html_gz_size = (index_html_gz_end - index_html_gz_start);
    if (index_html_gz_size) {
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
        httpd_resp_send(req, (const char *)index_html_gz_start, index_html_gz_size);
    } else {
        esp3d_log_e("Invalid ressource");
        httpd_resp_sendstr(req, "Cannot load icon from webserver");
    }

    return ESP_OK;
}
