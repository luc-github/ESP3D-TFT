/*
  esp3d_socket_server

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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "esp3d_client.h"
#include "esp3d_log.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ESP3D_MAX_SOCKET_CLIENTS 2

typedef struct {
    int socketId;
    struct sockaddr_storage source_addr;
#if ESP3D_AUTHENTICATION_FEATURE
    char sessionId[25];
#endif //#if ESP3D_AUTHENTICATION_FEATURE
} esp3d_socket_client_info_t;

class ESP3DSocketServer : public Esp3DClient
{
public:
    ESP3DSocketServer();
    ~ESP3DSocketServer();
    bool begin();
    void handle();
    void process(esp3d_msg_t * msg);
    void readSockets();
    void end();
    bool isEndChar(uint8_t ch);
    bool pushMsgToRxQueue(uint index, const uint8_t* msg, size_t size);
    void flush();
    bool started()
    {
        return _started;
    }
    uint32_t port()
    {
        return _port;
    }
    bool startSocketServer();
    bool getClient();
    uint clientsConnected();
    void closeAllClients();
    esp3d_socket_client_info_t * getClientInfo(uint index);

private:

    bool sendToSocket(const int sock, const char * data, const size_t len);
    bool closeSocket(int socketId);
    int getFreeClientSlot();
    esp3d_socket_client_info_t _clients[ESP3D_MAX_SOCKET_CLIENTS];
    int _listen_socket;
    bool _started;
    uint32_t _port;
    pthread_mutex_t _tx_mutex;
    pthread_mutex_t _rx_mutex;
    TaskHandle_t _xHandle;
    char ** _buffer;
    char * _data;
};

extern ESP3DSocketServer esp3dSocketServer;

#ifdef __cplusplus
} // extern "C"
#endif