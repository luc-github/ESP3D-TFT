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
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_commands.h"
#include  "authentication/esp3d_authentication.h"

esp_err_t Esp3DHttpService::config_handler(httpd_req_t *req)
{
    esp3d_log("Uri: %s", req->uri);
    esp3d_authentication_level_t  authentication_level = esp3dAuthenthicationService.getAuthenticatedLevel();
    esp3d_msg_t * newMsgPtr = Esp3DClient::newMsg( WEBUI_CLIENT, ESP3D_COMMAND, (const uint8_t *) "[ESP420]addPreTag",  strlen("[ESP420]addPreTag"), authentication_level);
    if (newMsgPtr) {
        newMsgPtr->requestId.httpReq = req;
        esp3dCommands.process(newMsgPtr);
        return ESP_OK;
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Message creation failed");
    }
    return ESP_FAIL;
}
