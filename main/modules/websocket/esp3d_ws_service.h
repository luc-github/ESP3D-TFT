/*
  esp3d_ws_service

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
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp3d_client.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_WS_CLIENTS 4

class Esp3DWsService final
{
public:
    Esp3DWsService();
    ~Esp3DWsService();
    bool begin(httpd_handle_t serverHandle);
    void handle();
    void end();
    void process(esp3d_msg_t * msg);
    esp_err_t process(httpd_req_t *req);
    esp_err_t onOpen(httpd_req_t *req);
    esp_err_t onMessage(httpd_req_t *req);
    esp_err_t onClose(int fd);
    void closeClients();
    void enableOnly (int fd);
    int getCurrentFd()
    {
        return _currentFd;
    }
    esp_err_t pushNotification(const char *msg);
    esp_err_t pushMsgTxt(const char *msg);
    esp_err_t pushMsgTxt(int fd, const char *msg);
    esp_err_t pushMsgTxt(int fd, uint8_t *msg, size_t len);
    esp_err_t pushMsgBin(uint8_t *msg, size_t len);
    esp_err_t pushMsgBin(int fd, uint8_t *msg, size_t len);
    esp_err_t BroadcastTxt(const char *msg, int ignore=-1);
    esp_err_t BroadcastTxt(uint8_t *msg, size_t len, int ignore=-1);
    esp_err_t BroadcastBin(uint8_t *msg, size_t len, int ignore=-1);
    bool started()
    {
        return _started;
    };

private:
    httpd_handle_t _server;
    httpd_req_t *_req;
    int _currentFd;
    bool _started;
};

extern Esp3DWsService esp3dWsWebUiService;
#ifdef __cplusplus
} // extern "C"
#endif