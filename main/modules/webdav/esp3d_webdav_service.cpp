/*
  esp3d_webdav_service
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

#include "esp3d_webdav_service.h"

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_version.h"
#include "http/esp3d_http_service.h"
#include "network/esp3d_network.h"
#include "sdkconfig.h"

esp_err_t httpd_resp_set_webdav_hdr(httpd_req_t *req) {
  esp_err_t err = ESP_OK;

  err = httpd_resp_set_hdr(req, "DAV", ESP3D_WEBDAV_HEADER);
  if (err != ESP_OK) {
    esp3d_log_e("httpd_resp_set_hdr failed for DAV");
    return err;
  }

  err = httpd_resp_set_hdr(req, "User-Agent",
                           "ESP3D-TFT-WebdavServer/1.0 (" CONFIG_IDF_TARGET
                           "; Firmware/" ESP3D_TFT_VERSION
                           "; Platform/RTOS; Embedded; http://www.esp3d.io)");
  if (err != ESP_OK) {
    esp3d_log_e("httpd_resp_set_hdr failed for User-Agent");
    return err;
  }

  static std::string host = "";
  if (host.empty()) {
    host = "http://";
    host += esp3dNetwork.getLocalIpString();
    if (esp3dHttpService.getPort() != 80) {
      host += ":";
      host += esp3dHttpService.getPort();
    }
    host += "/";
    host += ESP3D_WEBDAV_ROOT;
  }

  err = httpd_resp_set_hdr(req, "Host", host.c_str());
  if (err != ESP_OK) {
    esp3d_log_e("httpd_resp_set_hdr failed for Host");
    return err;
  }

  err = httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
  if (err != ESP_OK) {
    esp3d_log_e("httpd_resp_set_hdr failed for cache-control");
    return err;
  }

  err = httpd_resp_set_hdr(req, "Allow", ESP3D_WEBDAV_METHODS);
  if (err != ESP_OK) {
    esp3d_log_e("httpd_resp_set_hdr failed for Allow");
    return err;
  }

  err = httpd_resp_set_hdr(req, "Connection", "close");
  if (err != ESP_OK) {
    esp3d_log_e("httpd_resp_set_hdr failed for Connection");
    return err;
  }

  return err;
}