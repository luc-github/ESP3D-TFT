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
#include <esp_http_server.h>
#include <stdio.h>

#include "esp3d_client.h"
#include "http/esp3d_http_service.h"
#include "lwip/sockets.h"

#define ESP3D_WS_DATA_URL "/wsdata"
#define ESP3D_WS_DATA_SUBPROTOCOL "arduino"

#ifdef __cplusplus
extern "C" {
#endif

struct ESP3DWebSocketConfig {
  httpd_handle_t server_handle;
  uint max_clients;
  esp3dSocketType type;
};

struct ESP3DWebSocketInfos {
  int socket_id;
  struct sockaddr_storage source_addr;
  char *buffer;
  uint buf_position;
#if ESP3D_AUTHENTICATION_FEATURE
  char session_id[25];
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
};

class ESP3DWsService {
 public:
  ESP3DWsService();
  ~ESP3DWsService();
  bool begin(ESP3DWebSocketConfig *config);
  void handle();
  void end();
  esp_err_t http_handler(httpd_req_t *req);
  virtual void process(ESP3DMessage *msg);
  virtual esp_err_t onOpen(httpd_req_t *req);
  virtual esp_err_t onMessage(httpd_req_t *req);
  virtual esp_err_t onClose(int fd);
  virtual bool pushMsgToRxQueue(int socketId, const uint8_t *msg, size_t size);

  bool isEndChar(uint8_t ch);
  int getFreeClientIndex();
  uint clientsConnected();
  bool isConnected() { return clientsConnected() > 0; }
  ESP3DWebSocketInfos *getClientInfos(uint index);
  ESP3DWebSocketInfos *getClientInfosFromSocketId(int socketId);
  bool addClient(int socketid);
  void closeClients();
  bool closeClient(int socketId);
  esp_err_t pushMsgTxt(int fd, const char *msg);
  esp_err_t pushMsgTxt(int fd, uint8_t *msg, size_t len);
  esp_err_t pushMsgBin(int fd, uint8_t *msg, size_t len);
  esp_err_t BroadcastTxt(const char *msg, int ignore = -1);
  esp_err_t BroadcastTxt(uint8_t *msg, size_t len, int ignore = -1);
  esp_err_t BroadcastBin(uint8_t *msg, size_t len, int ignore = -1);
  bool started() { return _started; }
  uint maxClients() { return _max_clients; }
  esp3dSocketType type() { return _type; }

 private:
  httpd_handle_t _server;
  bool _started;
  ESP3DWebSocketInfos *_clients;
  uint _max_clients;
  esp3dSocketType _type;
};
#if ESP3D_WS_SERVICE_FEATURE
extern ESP3DWsService esp3dWsDataService;
#endif  // ESP3D_WS_SERVICE_FEATURE
#ifdef __cplusplus
}  // extern "C"
#endif