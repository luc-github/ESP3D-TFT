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
#include "esp3d_client.h"
#include <list>
#include <utility>

#ifdef __cplusplus
extern "C" {
#endif

//for file upload
typedef enum {
    upload_file_start,
    upload_file_write,
    upload_file_end,
    upload_file_aborted,
} esp3d_upload_state_t;

//for complete upload (several files in upload)
typedef enum {
    upload_not_started,
    upload_ongoing,
    upload_error,
    upload_success,
} esp3d_upload_status_t;

typedef enum {
//Errors code
    ESP3D_HTTP_NO_ERROR =0,
    ESP3D_HTTP_ERROR_AUTHENTICATION,
    ESP3D_HTTP_FILE_CREATION,
    ESP3D_HTTP_FILE_WRITE,
    ESP3D_HTTP_UPLOAD,
    ESP3D_HTTP_NOT_ENOUGH_SPACE,
    ESP3D_HTTP_START_UPDATE,
    ESP3D_HTTP_FILE_CLOSE,
    ESP3D_HTTP_NO_SD,
    ESP3D_HTTP_MOUNT_SD,
    ESP3D_HTTP_RESET_NUMBERING, //not used
    ESP3D_HTTP_MEMORY_ERROR,
    ESP3D_HTTP_ACCESS_ERROR,
    ESP3D_HTTP_SIZE,
    ESP3D_HTTP_UPDATE,

} esp3d_http_error_t;

typedef struct {
    esp_err_t (*writeFn)(const uint8_t * data, size_t datasize,esp3d_upload_state_t file_upload_state, const char * filename, size_t filesize);
    esp_err_t (*nextHandler)(httpd_req_t *req);
    uint packetReadSize;
    uint packetWriteSize;
    esp3d_upload_status_t status;
    std::list<std::pair<std::string, std::string>> args;
} post_upload_ctx_t;

class Esp3DHttpService final
{
public:
    Esp3DHttpService();
    ~Esp3DHttpService();
    bool begin();
    void handle();
    void end();
    void process(esp3d_msg_t * msg);
    bool started()
    {
        return _started;
    };
    httpd_handle_t getServerHandle()
    {
        return _server;
    };
    void pushError(esp3d_http_error_t errcode, const char * st);
    static esp_err_t root_get_handler(httpd_req_t *req);
    static esp_err_t command_handler(httpd_req_t *req);
    static esp_err_t config_handler(httpd_req_t *req);
    static esp_err_t description_xml_handler(httpd_req_t *req);
    static esp_err_t favicon_ico_handler(httpd_req_t *req);
    static esp_err_t file_not_found_handler(httpd_req_t *req, httpd_err_code_t err);
    static esp_err_t login_handler(httpd_req_t *req);
    static esp_err_t files_handler(httpd_req_t *req);
    static esp_err_t sdfiles_handler(httpd_req_t *req);
    static esp_err_t updatefw_handler(httpd_req_t *req);
    static esp_err_t websocket_handler(httpd_req_t *req);
    static esp_err_t post_multipart_handler(httpd_req_t *req);
    static esp_err_t upload_to_flash_handler(const uint8_t * data, size_t datasize,esp3d_upload_state_t file_upload_state, const char * filename, size_t filesize);
    static esp_err_t upload_to_sd_handler(const uint8_t * data, size_t datasize,esp3d_upload_state_t file_upload_state, const char * filename, size_t filesize);
    static esp_err_t upload_to_updatefw_handler(const uint8_t * data, size_t datasize,esp3d_upload_state_t file_upload_state, const char * filename, size_t filesize);
    //static esp_err_t open_fn(httpd_handle_t hd, int socketFd);
    //static void  close_fn(httpd_handle_t hd, int socketFd);
    esp_err_t streamFile (const char * path,httpd_req_t *req );
    esp_err_t sendStringChunk (httpd_req_t *req, const char * str, bool autoClose = true );
    esp_err_t sendBinaryChunk (httpd_req_t *req, const uint8_t * data, size_t len, bool autoClose = true );
    bool hasArg(httpd_req_t *req,const char* argname );
    const char * getArg(httpd_req_t *req, const char* argname);
private:
    bool _started;
    httpd_handle_t _server;
    const char * getBoundaryString (httpd_req_t *req);
    //it is based on : `only one post is supported at once`
    static post_upload_ctx_t _post_files_upload_ctx;
    static post_upload_ctx_t _post_sdfiles_upload_ctx;
    static post_upload_ctx_t _post_updatefw_upload_ctx;
};

extern Esp3DHttpService esp3dHttpService;
#ifdef __cplusplus
} // extern "C"
#endif