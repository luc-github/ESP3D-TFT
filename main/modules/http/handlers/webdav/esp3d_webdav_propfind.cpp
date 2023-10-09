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

esp_err_t ESP3DHttpService::webdav_propfind_handler(httpd_req_t *req) {
  esp3d_log("Uri: %s", req->uri);
  // TODO: Implement method PROPFIND
  // Get depth from header
  // check for if exists (error 404)
  // check if file or directory
  // if depth==0 or file just stat the file or directory
  // if depth>0 and directory respond with a list of files /directories in the
  // directory of one depth
  // return the information about the request response
  // the response is an xml
  // the response is a 207 status code
  // Content-Type: application/xml;charset=utf-8
  //<? xml version = "1.0" encoding = "utf-8" ?>
  //<multistatus xmlns = "DAV:" depth="1">
  //  <response>
  //    <href>/monrep/</href>
  //    <propstat>
  //      <prop>
  //        <getlastmodified> Tue,10 Jan 2023 09 : 00 : 00 GMT</getlastmodified>
  //        <resourcetype>
  //          <collection />
  //        </ resourcetype>
  //      </ prop>
  //      <status>HTTP/1.1 200 OK</ status>
  //    </propstat>
  //  </response>
  //
  //  <response>
  //    <href>/monrep/monsubrep/</ href>
  //    <propstat>
  //      <prop>
  //        <getlastmodified> Tue, 10 Jan 2023 09 : 05 : 00
  //        GMT</getlastmodified> <resourcetype>
  //          <collection />
  //        </resourcetype>
  //      </prop>
  //      <status>HTTP/1.1 200 OK</status>
  //    </propstat>
  //  </response>
  //
  //  <response>
  //    <href>/monrep/monfichier.txt</href>
  //    <propstat>
  //      <prop>
  //        <getlastmodified> Tue, 10 Jan 2023 09 : 10 : 00
  //        GMT</getlastmodified>
  //        <getcontentlength>3000000</getcontentlength>
  //        <resourcetype />
  //      </prop>
  //      <status> HTTP / 1.1 200 OK</status>
  //    </propstat>
  //  </response>
  //</multistatus>

  return ESP_OK;
}
