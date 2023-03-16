/*
  esp3d-client class base object

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

#include "esp3d_client.h"

#include <string.h>

#include <string>

#include "esp3d_log.h"
#include "esp_timer.h"

#if ESP3D_TFT_LOG
int32_t msg_counting = 0;
#endif  // ESP3D_TFT_LOG

Esp3DClient::Esp3DClient() {
  _rx_size = 0;
  _tx_size = 0;
  _rx_max_size = 1024;
  _tx_max_size = 1024;
  _rx_mutex = nullptr;
  _tx_mutex = nullptr;
}
bool Esp3DClient::clearRxQueue() {
  while (!_rx_queue.empty()) {
    deleteMsg(popRx());
  }
  _rx_size = 0;
  return true;
}

Esp3dMessage* Esp3DClient::popRx() {
  Esp3dMessage* msg = _rx_queue.front();
  _rx_size -= msg->size;
  _rx_queue.pop_front();
  return msg;
}
Esp3dMessage* Esp3DClient::popTx() {
  Esp3dMessage* msg = _tx_queue.front();
  _tx_size -= msg->size;
  _tx_queue.pop_front();
  return msg;
}

void Esp3DClient::deleteMsg(Esp3dMessage* msg) {
  if (msg) {
    // esp3d_log("Deletion origin: %d, Target: %d, size: %d  : Now we have %ld
    // msg", msg->origin, msg->target, msg->size, --msg_counting);
    free(msg->data);
    free(msg);
    msg = nullptr;
  }
}

bool Esp3DClient::clearTxQueue() {
  while (!_tx_queue.empty()) {
    deleteMsg(popTx());
  }
  // sanity check just in case
  _tx_size = 0;
  return true;
}
Esp3DClient::~Esp3DClient() {
  clearTxQueue();
  clearRxQueue();
}

bool Esp3DClient::addRxData(Esp3dMessage* msg) {
  bool res = false;
  if (_rx_mutex) {
    if (pthread_mutex_lock(_rx_mutex) == 0) {
      if (msg->size + _rx_size <= _rx_max_size) {
        _rx_queue.push_back(msg);
        _rx_size += msg->size;
        res = true;
      }
      pthread_mutex_unlock(_rx_mutex);
    }
  }
  return res;
}
bool Esp3DClient::addTxData(Esp3dMessage* msg) {
  bool res = false;
  if (_tx_mutex) {
    if (pthread_mutex_lock(_tx_mutex) == 0) {
      if (msg->size + _tx_size <= _tx_max_size) {
        _tx_queue.push_back(msg);
        _tx_size += msg->size;
        res = true;
      } else {
        esp3d_log_e("Queue Size limit exceeded %d vs %d", msg->size + _tx_size,
                    _tx_max_size);
      }
      pthread_mutex_unlock(_tx_mutex);
    }
  } else {
    esp3d_log_e("no mutex available");
  }
  return res;
}
bool Esp3DClient::addFrontTxData(Esp3dMessage* msg) {
  bool res = false;
  if (_tx_mutex) {
    if (pthread_mutex_lock(_tx_mutex) == 0) {
      if (msg->size + _tx_size <= _tx_max_size) {
        _tx_queue.push_front(msg);
        _tx_size += msg->size;
        res = true;
      }
      pthread_mutex_unlock(_tx_mutex);
    }
  }
  return res;
}

Esp3dMessage* Esp3DClient::newMsg() {
  Esp3dMessage* newMsgPtr = (Esp3dMessage*)malloc(sizeof(Esp3dMessage));
  if (newMsgPtr) {
    // esp3d_log("Creation : Now we have %ld msg", ++msg_counting);
    newMsgPtr->data = nullptr;
    newMsgPtr->size = 0;
    newMsgPtr->origin = Esp3dClientType::no_client;
    newMsgPtr->target = Esp3dClientType::all_clients;
    newMsgPtr->authentication_level = Esp3dAuthenticationLevel::guest;
    newMsgPtr->request_id.id = esp_timer_get_time();
    newMsgPtr->type = Esp3dMessageType::head;
  }
  return newMsgPtr;
}

Esp3dMessage* Esp3DClient::newMsg(Esp3dRequest requestId) {
  Esp3dMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = Esp3dClientType::command;
    newMsgPtr->request_id = requestId;
  }
  return newMsgPtr;
}

bool Esp3DClient::copyMsgInfos(Esp3dMessage* newMsgPtr, Esp3dMessage msg) {
  if (!newMsgPtr) {
    return false;
  }
  newMsgPtr->origin = msg.origin;
  newMsgPtr->target = msg.target;
  newMsgPtr->authentication_level = msg.authentication_level;
  newMsgPtr->request_id = msg.request_id;
  newMsgPtr->type = msg.type;
  return true;
}

Esp3dMessage* Esp3DClient::copyMsgInfos(Esp3dMessage msg) {
  Esp3dMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    copyMsgInfos(newMsgPtr, msg);
  }
  return newMsgPtr;
}

Esp3dMessage* Esp3DClient::copyMsg(Esp3dMessage msg) {
  Esp3dMessage* newMsgPtr = newMsg(msg.origin, msg.target, msg.data, msg.size,
                                   msg.authentication_level);
  if (newMsgPtr) {
    newMsgPtr->request_id = msg.request_id;
    newMsgPtr->type = msg.type;
  }
  return newMsgPtr;
}

Esp3dMessage* Esp3DClient::newMsg(
    Esp3dClientType origin, Esp3dClientType target, const uint8_t* data,
    size_t length, Esp3dAuthenticationLevel authentication_level) {
  Esp3dMessage* newMsgPtr = newMsg(origin, target, authentication_level);
  if (newMsgPtr) {
    if (!setDataContent(newMsgPtr, data, length)) {
      deleteMsg(newMsgPtr);
      newMsgPtr = nullptr;
    }
  }
  return newMsgPtr;
}

Esp3dMessage* Esp3DClient::newMsg(
    Esp3dClientType origin, Esp3dClientType target,
    Esp3dAuthenticationLevel authentication_level) {
  Esp3dMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = origin;
    newMsgPtr->target = target;
    newMsgPtr->authentication_level = authentication_level;
  }
  return newMsgPtr;
}

bool Esp3DClient::setDataContent(Esp3dMessage* msg, const uint8_t* data,
                                 size_t length) {
  if (!msg) {
    esp3d_log_e("no valid msg container");
    return false;
  }
  if (!data || length == 0) {
    esp3d_log_e("no data to set");
    return false;
  }
  if (msg->data) {
    free(msg->data);
  }
  msg->data = (uint8_t*)malloc(sizeof(uint8_t) * length);
  if (msg->data) {
    memcpy(msg->data, data, length);
    msg->size = length;
    return true;
  }
  esp3d_log_e("Out of memory");
  return false;
}
