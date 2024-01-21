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
#if ESP3D_CAMERA_FEATURE
#include "authentication/esp3d_authentication.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "http/esp3d_http_service.h"
#include "camera/camera.h"
esp_err_t ESP3DHttpService::snap_handler(httpd_req_t *req)
{
  esp3d_log("Uri: %s", req->uri);
  ESP3DAuthenticationLevel authentication_level = getAuthenticationLevel(req);
  // Send httpd header
  httpd_resp_set_http_hdr(req);
#if ESP3D_AUTHENTICATION_FEATURE
  if (authentication_level == ESP3DAuthenticationLevel::guest)
  {
    // send 401
    return not_authenticated_handler(req);
  }
#endif // #if ESP3D_AUTHENTICATION_FEATURE

  esp3d_camera.handle_snap(req);
  return ESP_OK;
}
#endif // ESP3D_CAMERA_FEATURE