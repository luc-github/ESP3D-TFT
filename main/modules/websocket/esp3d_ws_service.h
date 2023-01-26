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
#include "lwip/sockets.h"
#include "esp3d_client.h"
#include "http/esp3d_http_service.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    httpd_handle_t serverHandle;
    uint max_clients;
    esp3d_http_socket_t type;
} esp3d_websocket_config_t;

typedef struct {
    int socketId;
    struct sockaddr_storage source_addr;
    char * buffer;
    uint bufPos;
#if ESP3D_AUTHENTICATION_FEATURE
    char sessionId[25];
#endif //#if ESP3D_AUTHENTICATION_FEATURE
} esp3d_ws_client_info_t;

class Esp3DWsService
{
public:
    Esp3DWsService();
    ~Esp3DWsService();
    bool begin(esp3d_websocket_config_t * config );
    void handle();
    void end();
    esp_err_t http_handler(httpd_req_t *req);
    virtual void process(esp3d_msg_t * msg);
    virtual esp_err_t onOpen(httpd_req_t *req);
    virtual esp_err_t onMessage(httpd_req_t *req);
    virtual esp_err_t onClose(int fd);
    virtual bool pushMsgToRxQueue(int socketId,const uint8_t *msg, size_t size);

    bool isEndChar(uint8_t ch);
    int getFreeClientIndex();
    uint  clientsConnected();
    esp3d_ws_client_info_t * getClientInfo(uint index);
    esp3d_ws_client_info_t * getClientInfoFromSocketId(int socketId);
    bool addClient(int socketid);
    void closeClients();
    bool closeClient(int socketId);
    esp_err_t pushMsgTxt(int fd, const char *msg);
    esp_err_t pushMsgTxt(int fd, uint8_t *msg, size_t len);
    esp_err_t pushMsgBin(int fd, uint8_t *msg, size_t len);
    esp_err_t BroadcastTxt(const char *msg, int ignore=-1);
    esp_err_t BroadcastTxt(uint8_t *msg, size_t len, int ignore=-1);
    esp_err_t BroadcastBin(uint8_t *msg, size_t len, int ignore=-1);
    bool started()
    {
        return _started;
    }
    uint maxClients()
    {
        return _max_clients;
    }
    esp3d_http_socket_t type()
    {
        return _type;
    }

private:
    httpd_handle_t _server;
    bool _started;
    esp3d_ws_client_info_t * _clients;
    uint _max_clients;
    esp3d_http_socket_t _type;
};

extern Esp3DWsService  esp3dWsDataService;

#ifdef __cplusplus
} // extern "C"
#endif