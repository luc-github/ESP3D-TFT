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

// WebDav Light for esp32
// * no etag support because mcu consuming parsing all files,
//   especially big gcodes files and getlastmodified is supposed to be a backup
//   check if not etag, and etag is not not mandatory
// * no lock support because we are in mono user mode, and it is not mandatory
// * always answer depth=0 what ever the request is, it make no sense to go
// deeper on esp32
// * put strict minmum properties in propfind whatt ever the request is
//   resourcetype, getcontenttype, getcontentlength (mandatories), and
//   getlastmodified (optional) to replace etag
// * no Accept-Ranges:bytes support because when esp32 is streaming content the
// origin (e.g:sd) is locked as the access is exclusivve, and no other action is
// suposed to be done, so it make no sense to allow to split the file.

// For a file:
// <D:response>
//   <D:href>/monfichier.txt</D:href>
//   <D:propstat>
//     <D:prop>
//       <D:resourcetype />
//       <D:getcontenttype>text/plain</D:getcontenttype>
//       <D:>getcontentlength> 2048 </D:getcontentlength>
//       <D:>getlastmodified>Sun, 06 Nov 1994 08:49:37 GMT</D:getlastmodified>
//     </D prop>
//     <D:status> HTTP / 1.1 200 OK</D:status>
//  </D:propstat>
//</D:response>

// For a folder:
// <D:response>
//   <D:href>/myfolder</D:href>
//   <D:propstat>
//     <D:prop>
//       <D:resourcetype><D:collection/></D:resourcetype>
//       <D:getlastmodified>Sun, 06 Nov 1994 08:49:37 GMT</D:getlastmodified>
//     </D:prop>
//     <D:status>HTTP/1.1 200 OK</D:status>
//   </D:propstat>

//<D:depth>0</D:depth>

// In theory there is  need to do more and it is still compliant with the
// standard RFC 4918, per claude instant ia and copilot ia ^_^
// This should also to avoid the need to parse the payload of the request
// because we won't care about it

esp_err_t httpd_resp_set_webdav_hdr(httpd_req_t *req) {
  esp_err_t err = ESP_OK;
  err = httpd_resp_set_hdr(req, "DAV", ESP3D_WEBDAV_HEADER);
  if (err != ESP_OK) {
    esp3d_log_e("httpd_resp_set_hdr failed for DAV");
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
  return err;
}