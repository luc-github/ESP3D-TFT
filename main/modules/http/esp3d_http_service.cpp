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
#include "tasks_def.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "esp3d_commands.h"
#include "network/esp3d_network.h"
#include "filesystem/esp3d_globalfs.h"

#define CHUNK_BUFFER_SIZE STREAM_CHUNK_SIZE
char chunk[CHUNK_BUFFER_SIZE];

Esp3DHttpService esp3dHttpService;

Esp3DHttpService::Esp3DHttpService()
{
    _started = false;
    _server = nullptr;
}

Esp3DHttpService::~Esp3DHttpService() {}


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
        //favicon.ico
        const httpd_uri_t favicon_handler_config = {
            .uri       = "/favicon.ico",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.favicon_ico_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &favicon_handler_config);
        //root /
        const httpd_uri_t root_handler_config = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.root_get_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &root_handler_config);
        //Command /command
        const httpd_uri_t command_handler_config = {
            .uri       = "/command",
            .method    = HTTP_GET,
            .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dHttpService.command_handler),
            .user_ctx  =  nullptr,
            .is_websocket = false,
            .handle_ws_control_frames = false,
            .supported_subprotocol = nullptr
        };
        httpd_register_uri_handler(_server, &command_handler_config);
        //File not found
        httpd_register_err_handler(_server, HTTPD_404_NOT_FOUND, (httpd_err_handler_func_t )file_not_found_handler);
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
        httpd_unregister_uri(_server, "/favicon.ico");
        httpd_unregister_uri(_server, "/command");
        httpd_unregister_uri(_server, "/");
        httpd_register_err_handler(_server, HTTPD_404_NOT_FOUND, NULL);
        httpd_stop(_server);
    }
    _server = nullptr;
    _started = false;
}


esp_err_t Esp3DHttpService::streamFile (const char * path,httpd_req_t *req )
{
    esp_err_t res = ESP_OK;
    if (!_started|| !_server) {
        esp3d_log_e("Stream server is not ready");
        return ESP_ERR_INVALID_STATE;
    }
    std::string filename = "";
    std::string filenameGz = "";
    bool isGzip = false;
    //check if filename is provided or need to extract from request string
    if (path) {  // this one is already correct no need to decode it neither append mount point
        filename = path;
        filenameGz = filename+".gz";
    } else { //extract file name from query and decode it
        size_t buf_len = httpd_req_get_url_query_len(req) + 1;
        char * buf = (char *)malloc(buf_len);
        if (buf) {
            res = httpd_req_get_url_query_str(req, buf, buf_len);
            if (res != ESP_OK) {
                esp3d_log_e("Cannot extract query string from uri: %s", esp_err_to_name(res));
            } else {
                //clear possible parameters
                for (uint i = 0; i < buf_len; i++) {
                    if (buf[i] == '?') {
                        buf[i]=0x0;
                        break;
                    }
                }
                esp3d_fs_types fstype = globalFs.getFSType(buf);
                //assume default file serving is flash
                if (fstype==FS_UNKNOWN) {
                    filename=globalFs.mount_point(FS_FLASH);
                }
                filename += esp3d_strings::urlDecode((const char *)buf);
                filenameGz = filename+".gz";
            }
            free(buf );
        } else {
            esp3d_log_e("Memory allocation failed");
            res= ESP_ERR_NO_MEM;
        }
    }
    if (res== ESP_OK) {
        esp3d_log("File name is %s", filename.c_str());
        if (globalFs.accessFS(filename.c_str())) {
            if (globalFs.exists(filenameGz.c_str())) {
                esp3d_log("File exists and it is gzipped");
                isGzip = true;
                res = ESP_OK;
            } else if (globalFs.exists(filename.c_str())) {
                esp3d_log("File exists");
                res = ESP_OK;
            } else {
                esp3d_log_e("File %s does not exists", filename.c_str());
                res = ESP_ERR_NOT_FOUND;
            }
            if (res == ESP_OK) {
                FILE *fd = globalFs.open(filename.c_str(), "r");
                if (fd) {
                    //stream file
                    std::string mimeType= esp3d_strings::getContentType( filename.c_str());

                    httpd_resp_set_type(req, mimeType.c_str());
                    if(isGzip) {
                        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
                    }
                    size_t chunksize;
                    do {
                        chunksize = fread(chunk, 1, CHUNK_BUFFER_SIZE, fd);
                        if (chunksize > 0) {
                            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                                esp3d_log_e("File sending failed!");
                                chunksize = 0;
                                res = ESP_FAIL;
                            }
                        }
                    } while (chunksize != 0);
                    fclose(fd);
                    httpd_resp_send_chunk(req, NULL, 0);
                } else {
                    res = ESP_ERR_NOT_FOUND;
                    esp3d_log_e("Cannot access File");
                }

            }
            globalFs.releaseFS(filename.c_str());
        } else {
            res = ESP_ERR_NOT_FOUND;
            esp3d_log_e("Cannot access FS");
        }
    }

    return res;
}

void Esp3DHttpService::process(esp3d_msg_t * msg)
{
    if (msg->requestId.httpReq) {
        esp3d_log("Msg type : %d", msg->type);
        if (httpd_resp_send_chunk(msg->requestId.httpReq, (const char *)msg->data, msg->size) != ESP_OK) {
            httpd_resp_send_chunk(msg->requestId.httpReq, NULL, 0);
            esp3d_log("Error sending data");
        } else {
            if (msg->type==  msg_tail || msg->type== msg_unique) {
                httpd_resp_send_chunk(msg->requestId.httpReq, NULL, 0);
            }
        }
    }
    Esp3DClient::deleteMsg(msg);
}