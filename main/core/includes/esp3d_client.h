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

union Esp3dRequest {
  uint id;
  httpd_req_t *http_request;
};

enum class Esp3dMessageType : uint8_t { head, core, tail, unique };

struct Esp3dMessage {
  uint8_t *data;
  size_t size;
  Esp3dClientType origin;
  Esp3dClientType target;
  Esp3dAuthenticationLevel authentication_level;
  Esp3dRequest request_id;
  Esp3dMessageType type;
};

class ESP3dClient {
 public:
  ESP3dClient();
  ~ESP3dClient();
  virtual bool begin() { return false; };
  virtual void handle(){};
  virtual void end(){};
  virtual void flush(){};
  void setTxMaxSize(size_t max) { _tx_max_size = max; };
  void setRxMaxSize(size_t max) { _rx_max_size = max; };
  bool addRxData(Esp3dMessage *msg);
  bool addTxData(Esp3dMessage *msg);
  Esp3dMessage *popRx();
  Esp3dMessage *popTx();
  static void deleteMsg(Esp3dMessage *msg);
  bool addFrontTxData(Esp3dMessage *msg);
  void setRxMutex(pthread_mutex_t *mutex) { _rx_mutex = mutex; };
  void setTxMutex(pthread_mutex_t *mutex) { _tx_mutex = mutex; };
  bool clearRxQueue();
  bool clearTxQueue();
  size_t getRxMsgsCount() { return _rx_queue.size(); }
  size_t getTxMsgsCount() { return _tx_queue.size(); }
  static Esp3dMessage *copyMsg(Esp3dMessage msg);
  static Esp3dMessage *copyMsgInfos(Esp3dMessage msg);
  static bool copyMsgInfos(Esp3dMessage *newMsgPtr, Esp3dMessage msg);
  static Esp3dMessage *newMsg();
  static Esp3dMessage *newMsg(Esp3dRequest requestId);
  static Esp3dMessage *newMsg(Esp3dClientType origin, Esp3dClientType target,
                              Esp3dAuthenticationLevel authentication_level =
                                  Esp3dAuthenticationLevel::guest);
  static Esp3dMessage *newMsg(Esp3dClientType origin, Esp3dClientType target,
                              const uint8_t *data, size_t length,
                              Esp3dAuthenticationLevel authentication_level =
                                  Esp3dAuthenticationLevel::guest);
  static bool setDataContent(Esp3dMessage *msg, const uint8_t *data,
                             size_t length);

 private:
  std::deque<Esp3dMessage *> _rx_queue;
  std::deque<Esp3dMessage *> _tx_queue;
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