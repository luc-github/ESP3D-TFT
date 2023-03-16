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

union ESP3DRequest {
  uint id;
  httpd_req_t *http_request;
};

enum class ESP3DMessageType : uint8_t { head, core, tail, unique };

struct ESP3DMessage {
  uint8_t *data;
  size_t size;
  ESP3DClientType origin;
  ESP3DClientType target;
  ESP3DAuthenticationLevel authentication_level;
  ESP3DRequest request_id;
  ESP3DMessageType type;
};

class ESP3DClient {
 public:
  ESP3DClient();
  ~ESP3DClient();
  virtual bool begin() { return false; };
  virtual void handle(){};
  virtual void end(){};
  virtual void flush(){};
  void setTxMaxSize(size_t max) { _tx_max_size = max; };
  void setRxMaxSize(size_t max) { _rx_max_size = max; };
  bool addRxData(ESP3DMessage *msg);
  bool addTxData(ESP3DMessage *msg);
  ESP3DMessage *popRx();
  ESP3DMessage *popTx();
  static void deleteMsg(ESP3DMessage *msg);
  bool addFrontTxData(ESP3DMessage *msg);
  void setRxMutex(pthread_mutex_t *mutex) { _rx_mutex = mutex; };
  void setTxMutex(pthread_mutex_t *mutex) { _tx_mutex = mutex; };
  bool clearRxQueue();
  bool clearTxQueue();
  size_t getRxMsgsCount() { return _rx_queue.size(); }
  size_t getTxMsgsCount() { return _tx_queue.size(); }
  static ESP3DMessage *copyMsg(ESP3DMessage msg);
  static ESP3DMessage *copyMsgInfos(ESP3DMessage msg);
  static bool copyMsgInfos(ESP3DMessage *newMsgPtr, ESP3DMessage msg);
  static ESP3DMessage *newMsg();
  static ESP3DMessage *newMsg(ESP3DRequest requestId);
  static ESP3DMessage *newMsg(ESP3DClientType origin, ESP3DClientType target,
                              ESP3DAuthenticationLevel authentication_level =
                                  ESP3DAuthenticationLevel::guest);
  static ESP3DMessage *newMsg(ESP3DClientType origin, ESP3DClientType target,
                              const uint8_t *data, size_t length,
                              ESP3DAuthenticationLevel authentication_level =
                                  ESP3DAuthenticationLevel::guest);
  static bool setDataContent(ESP3DMessage *msg, const uint8_t *data,
                             size_t length);

 private:
  std::deque<ESP3DMessage *> _rx_queue;
  std::deque<ESP3DMessage *> _tx_queue;
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