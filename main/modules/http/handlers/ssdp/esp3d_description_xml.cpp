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

#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp_wifi.h"
#include "http/esp3d_http_service.h"
#include "ssdp/esp3d_ssdp.h"

esp_err_t ESP3DHttpService::description_xml_handler(httpd_req_t *req) {
  // No authentication for this URL
  // Send httpd header
  httpd_resp_set_http_hdr(req);
  esp3d_log("Uri: %s", req->uri);
  httpd_resp_set_type(req, "text/xml");
  const char *response = esp3d_ssdp_service.get_schema();
  return httpd_resp_send(req, response, strlen(response));
}
