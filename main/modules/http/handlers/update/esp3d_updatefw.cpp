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
#include "esp3d_hal.h"
#include "esp3d_string.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "http/esp3d_http_service.h"

esp_err_t ESP3DHttpService::updatefw_handler(httpd_req_t *req) {
  // No need Authentication as already handled in multipart_parser
  esp3d_log("Uri: %s", req->uri);
  // Send httpd header
  httpd_resp_set_http_hdr(req);
  httpd_resp_sendstr(req, "Update firmware done restarting");
  esp3d_hal::wait(1000);
  esp_restart();
  return ESP_OK;
}
