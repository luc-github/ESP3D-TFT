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

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "http/esp3d_http_service.h"

esp_err_t ESP3DHttpService::root_get_handler(httpd_req_t *req) {
  // not blocking
#if ESP3D_AUTHENTICATION_FEATURE
  ESP3DAuthenticationLevel level = getAuthenticationLevel(req);
  esp3d_log("Uri: %s, %d", req->uri, static_cast<uint8_t>(level));
  (void)level;
#endif  // ESP3D_AUTHENTICATION_FEATURE
  // Send httpd header
  httpd_resp_set_http_hdr(req);
  bool forcefallback = false;
  size_t buf_len = httpd_req_get_url_query_len(req);
  if (buf_len > 0) {
    char *buf;
    buf_len++;  // for 0x0
    buf = (char *)malloc(buf_len);
    memset(buf, 0, buf_len);
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      char value[255] = {0};
      esp3d_log("query string: %s", buf);
      if (httpd_query_key_value(buf, "forcefallback", value, 254) == ESP_OK) {
        std::string forcefallbacksval = esp3d_string::urlDecode(value);
        esp3d_string::str_toUpperCase(&forcefallbacksval);
        esp3d_log("forcefallback value: %s", forcefallbacksval.c_str());

        if (forcefallbacksval == "TRUE" || forcefallbacksval == "1" ||
            forcefallbacksval == "YES") {
          forcefallback = true;
          esp3d_log("forcefallback enabled");
        }
      } else {
        esp3d_log("Invalid param");
      }
    }
    free(buf);
  }
  esp_err_t err = ESP_ERR_NOT_FOUND;
  if (!forcefallback) {
    err = esp3dHttpService.streamFile("/fs/index.html", req);
  }
  if (err == ESP_ERR_NOT_FOUND) {
    esp3d_log("Use embedded index.html.gz");
    extern const unsigned char index_html_gz_start[] asm(
        "_binary_index_html_gz_start");
    extern const unsigned char index_html_gz_end[] asm(
        "_binary_index_html_gz_end");
    const size_t index_html_gz_size = (index_html_gz_end - index_html_gz_start);
    if (index_html_gz_size) {
      httpd_resp_set_type(req, "text/html");
      httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
      err = httpd_resp_send(req, (const char *)index_html_gz_start,
                            index_html_gz_size);
    } else {
      esp3d_log_e("Invalid ressource");
      err = ESP_FAIL;
    }
  }
  if (err != ESP_OK) {
    esp3d_log_e("Cannot serve file: %s", esp_err_to_name(err));
  }
  // success or fail ?
  return err == ESP_OK ? ESP_OK : ESP_FAIL;
}
