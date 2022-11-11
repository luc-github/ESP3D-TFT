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


#include "esp3d_http_service.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp3d_commands.h"
#include "network/esp3d_network.h"
Esp3DHttpService esp3dHttpService;

Esp3DHttpService::Esp3DHttpService()
{
    _started = false;
    _server = nullptr;
}

Esp3DHttpService::~Esp3DHttpService() {}

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    /* if (strcmp("/hello", req->uri) == 0) {
         httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
         // Return ESP_OK to keep underlying socket open
         return ESP_OK;
     } else if (strcmp("/echo", req->uri) == 0) {
         httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
         // Return ESP_FAIL to close underlying socket
         return ESP_FAIL;
     }*/
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 - File not found");
    return ESP_FAIL;
}


const httpd_uri_t root_handler_config = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.root_get_handler),
    .user_ctx  =  (void *)"Hello World!",
    //.is_websocket = false,
    //.handle_ws_control_frames = false,
    // .supported_subprotocol = nullptr
};


bool Esp3DHttpService::begin()
{
    esp3d_log("Starting Http Service");

    end();
    //check if start
    if (esp3d_state_on!= (esp3d_state_t)esp3dTFTsettings.readByte(esp3d_http_on)) {
        esp3d_log("Http is not enabled");
        //return true because no error but _started is false
        return true;
    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    uint32_t  intValue = esp3dTFTsettings.readUint32(esp3d_http_port);
    //HTTP port
    config.server_port = intValue;
    //Http server core
    config.core_id = 0;
    //start server
    esp3d_log("Starting server on port: '%d'", config.server_port);
    if (httpd_start(&_server, &config) == ESP_OK) {
        // Set URI handlers
        esp3d_log("Registering URI handlers");
        httpd_register_uri_handler(_server, &root_handler_config);
        httpd_register_err_handler(_server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        _started = true;
    }
    return _started;
}

void Esp3DHttpService::handle() {}

void Esp3DHttpService::end()
{
    if (!_started && !_server) {
        return;
    }
    esp3d_log("Stop Http Service");
    if (_server) {
        httpd_unregister_uri(_server, "/");
        httpd_register_err_handler(_server, HTTPD_404_NOT_FOUND, NULL);
        httpd_stop(_server);
    }
    _server = nullptr;
    _started = false;
}
