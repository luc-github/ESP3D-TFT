/*
  esp3d-client

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
#include <pthread.h>
#include <stdio.h>

#include <deque>

#include "authentication/esp3d_authentication_types.h"
#include "esp3d_client_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
  uint id;
  httpd_req_t *httpReq;
} esp3d_request_t;

enum class Esp3dMessageType : uint8_t { head, core, tail, unique };

typedef struct {
  uint8_t *data;
  size_t size;
  Esp3dClient origin;
  Esp3dClient target;
  Esp3dAuthenticationLevel authentication_level;
  esp3d_request_t requestId;
  Esp3dMessageType type;
} esp3d_msg_t;

class Esp3DClient {
 public:
  Esp3DClient();
  ~Esp3DClient();
  virtual bool begin() { return false; };
  virtual void handle(){};
  virtual void end(){};
  virtual void flush(){};
  void setTxMaxSize(size_t max) { _tx_max_size = max; };
  void setRxMaxSize(size_t max) { _rx_max_size = max; };
  bool addRXData(esp3d_msg_t *msg);
  bool addTXData(esp3d_msg_t *msg);
  esp3d_msg_t *popRx();
  esp3d_msg_t *popTx();
  static void deleteMsg(esp3d_msg_t *msg);
  bool addFrontTXData(esp3d_msg_t *msg);
  void setRxMutex(pthread_mutex_t *mutex) { _rx_mutex = mutex; };
  void setTxMutex(pthread_mutex_t *mutex) { _tx_mutex = mutex; };
  bool clearRxQueue();
  bool clearTxQueue();
  size_t getRxMsgsCount() { return _rx_queue.size(); }
  size_t getTxMsgsCount() { return _tx_queue.size(); }
  static esp3d_msg_t *copyMsg(esp3d_msg_t msg);
  static esp3d_msg_t *copyMsgInfos(esp3d_msg_t msg);
  static bool copyMsgInfos(esp3d_msg_t *newMsgPtr, esp3d_msg_t msg);
  static esp3d_msg_t *newMsg();
  static esp3d_msg_t *newMsg(esp3d_request_t requestId);
  static esp3d_msg_t *newMsg(Esp3dClient origin, Esp3dClient target,
                             Esp3dAuthenticationLevel authentication_level =
                                 Esp3dAuthenticationLevel::guest);
  static esp3d_msg_t *newMsg(Esp3dClient origin, Esp3dClient target,
                             const uint8_t *data, size_t length,
                             Esp3dAuthenticationLevel authentication_level =
                                 Esp3dAuthenticationLevel::guest);
  static bool setDataContent(esp3d_msg_t *msg, const uint8_t *data,
                             size_t length);

 private:
  std::deque<esp3d_msg_t *> _rx_queue;
  std::deque<esp3d_msg_t *> _tx_queue;
  size_t _rx_size;
  size_t _tx_size;
  size_t _rx_max_size;
  size_t _tx_max_size;
  pthread_mutex_t *_rx_mutex;
  pthread_mutex_t *_tx_mutex;
};

#ifdef __cplusplus
}  // extern "C"
#endif