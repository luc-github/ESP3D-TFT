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

esp_err_t ESP3DHttpService::webdav_copy_handler(httpd_req_t *req) {
  esp3d_log("Uri: %s", req->uri);
  // TODO: implement method COPY
  // extract full path from uri
  // extract path from uri
  // get destination fullpath from header Destination
  // get depth copy from header depth (0,1, infinity), if not header - only
  // depth=0
  // extract dest_path from fullpath destination get parameters from get
  // header Overwrite (F: T)= (false: true) clear payload from request if any
  // Check can access (error code 503)
  // check file size, if not enough space (error code 507)
  // check if file exists and check Overwrite value
  // if destination exists and Overwrite is false (error code 409)
  // copy file to dest_path
  //  close file
  //  release access
  //  response code 201 if success and new file
  //  response code 204 if success and overwrite file
  return ESP_OK;
}