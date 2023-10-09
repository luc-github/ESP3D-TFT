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

esp_err_t ESP3DHttpService::webdav_get_handler(httpd_req_t *req) {
  esp3d_log("Uri: %s", req->uri);
  // TODO: implement method GET
  // extract path from uri
  // clear payload from request if any
  // Check can access (error code 503)
  // Check if file exist(error code 404)
  // Check if file is a directory
  // if file send content-type and content-length header
  // if directory, send 200 response and return
  // read file and send it (error code 500 if any error)
  // close file
  // release access
  // response code 200 if success

  return ESP_OK;
}
