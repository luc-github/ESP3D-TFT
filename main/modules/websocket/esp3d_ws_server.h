/*
  esp3d_ws_server

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
#include "lwip/sockets.h"
#include "esp3d_client.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESP3D_MAX_WS_CLIENTS 1

typedef struct {
    int socketId;
    struct sockaddr_storage source_addr;
} esp3d_ws_client_info_t;

class ESP3DWsServer : public Esp3DClient
{
public:
    ESP3DWsServer();
    ~ESP3DWsServer();
    bool begin();
    void handle();
    void process(esp3d_msg_t * msg);
    void end();
    bool isEndChar(uint8_t ch);
    bool pushMsgToRxQueue(const uint8_t* msg, size_t size);
    void flush();
    bool started()
    {
        return _started;
    }
    uint32_t port()
    {
        return _port;
    }
    esp_err_t onOpen(httpd_req_t *req);
    esp_err_t onMessage(httpd_req_t *req);
    esp_err_t onClose(int fd);
    static void close_fn(httpd_handle_t hd, int socketFd);
    bool getClient();
    uint clientsConnected();
    void closeAllClients();
    esp3d_ws_client_info_t * getClientInfo(uint index);
    static esp_err_t websocket_handler(httpd_req_t *req);
private:
    void readSockets();
    bool sendToSocket(const int sock, const char * data, const size_t len);
    bool closeSocket(int socketId);
    int getFreeClientSlot();
    esp3d_ws_client_info_t _clients[ESP3D_MAX_WS_CLIENTS];
    bool _started;
    uint32_t _port;
    pthread_mutex_t _tx_mutex;
    pthread_mutex_t _rx_mutex;
    TaskHandle_t _xHandle;
    httpd_handle_t _server;
    char ** _buffer;
    char * _data;
};

extern ESP3DWsServer esp3dWsServer;

#ifdef __cplusplus
} // extern "C"
#endif