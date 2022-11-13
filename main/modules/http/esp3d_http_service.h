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

#pragma once
#include <stdio.h>
#include <esp_http_server.h>


#ifdef __cplusplus
extern "C" {
#endif


class Esp3DHttpService final
{
public:
    Esp3DHttpService();
    ~Esp3DHttpService();
    bool begin();
    void handle();
    void end();

    bool started()
    {
        return _started;
    };
    static esp_err_t root_get_handler(httpd_req_t *req);
    static esp_err_t command_handler(httpd_req_t *req);
    static esp_err_t config_handler(httpd_req_t *req);
    static esp_err_t description_xml_handler(httpd_req_t *req);
    static esp_err_t favicon_ico_handler(httpd_req_t *req);
    static esp_err_t file_not_found_handler(httpd_req_t *req, httpd_err_code_t err);
    static esp_err_t login_handler(httpd_req_t *req);
    static esp_err_t files_handler(httpd_req_t *req);
    static esp_err_t upload_files_handler(httpd_req_t *req);
    static esp_err_t sd_handler(httpd_req_t *req);
    static esp_err_t sdfiles_handler(httpd_req_t *req);
    static esp_err_t upload_sdfiles_handler(httpd_req_t *req);
    static esp_err_t updatefw_handler(httpd_req_t *req);
    static esp_err_t upload_updatefw_handler(httpd_req_t *req);
private:
    bool _started;
    httpd_handle_t _server;
};

extern Esp3DHttpService esp3dHttpService;
#ifdef __cplusplus
} // extern "C"
#endif