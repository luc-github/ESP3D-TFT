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

ESP3DClient::ESP3DClient() {
  _rx_size = 0;
  _tx_size = 0;
  _rx_max_size = 1024;
  _tx_max_size = 1024;
  _rx_mutex = nullptr;
  _tx_mutex = nullptr;
}
bool ESP3DClient::clearRxQueue() {
  while (!_rx_queue.empty()) {
    deleteMsg(popRx());
  }
  _rx_size = 0;
  return true;
}

ESP3DMessage* ESP3DClient::popRx() {
  ESP3DMessage* msg = nullptr;
  if (_rx_mutex) {
    if (pthread_mutex_lock(_rx_mutex) == 0) {
      msg = _rx_queue.front();
      _rx_size -= msg->size;
      _rx_queue.pop_front();
      pthread_mutex_unlock(_rx_mutex);
    }
  } else {
    esp3d_log_e("no mutex available");
  }
  return msg;
}
ESP3DMessage* ESP3DClient::popTx() {
  ESP3DMessage* msg = nullptr;
  if (_tx_mutex) {
    if (pthread_mutex_lock(_tx_mutex) == 0) {
      msg = _tx_queue.front();
      _tx_size -= msg->size;
      _tx_queue.pop_front();
      pthread_mutex_unlock(_tx_mutex);
    }
  } else {
    esp3d_log_e("no mutex available");
  }
  return msg;
}

void ESP3DClient::deleteMsg(ESP3DMessage* msg) {
  if (msg) {
    // esp3d_log("Deletion origin: %d, Target: %d, size: %d  : Now we have
    // %ld msg", msg->origin, msg->target, msg->size, --msg_counting);
    free(msg->data);
    free(msg);
    msg = nullptr;
  }
}

bool ESP3DClient::clearTxQueue() {
  while (!_tx_queue.empty()) {
    deleteMsg(popTx());
  }
  // sanity check just in case
  _tx_size = 0;
  return true;
}
ESP3DClient::~ESP3DClient() {
  clearTxQueue();
  clearRxQueue();
}

bool ESP3DClient::addRxData(ESP3DMessage* msg) {
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
bool ESP3DClient::addTxData(ESP3DMessage* msg) {
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
bool ESP3DClient::addFrontTxData(ESP3DMessage* msg) {
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

ESP3DMessage* ESP3DClient::newMsg() {
  ESP3DMessage* newMsgPtr = (ESP3DMessage*)malloc(sizeof(ESP3DMessage));
  if (newMsgPtr) {
    // esp3d_log("Creation : Now we have %ld msg", ++msg_counting);
    newMsgPtr->data = nullptr;
    newMsgPtr->size = 0;
    newMsgPtr->origin = ESP3DClientType::no_client;
    newMsgPtr->target = ESP3DClientType::all_clients;
    newMsgPtr->authentication_level = ESP3DAuthenticationLevel::guest;
    newMsgPtr->request_id.id = esp_timer_get_time();
    newMsgPtr->type = ESP3DMessageType::head;
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DClient::newMsg(ESP3DRequest requestId) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = ESP3DClientType::command;
    newMsgPtr->request_id = requestId;
  }
  return newMsgPtr;
}

bool ESP3DClient::copyMsgInfos(ESP3DMessage* newMsgPtr, ESP3DMessage msg) {
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

ESP3DMessage* ESP3DClient::copyMsgInfos(ESP3DMessage msg) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    copyMsgInfos(newMsgPtr, msg);
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DClient::copyMsg(ESP3DMessage msg) {
  ESP3DMessage* newMsgPtr = newMsg(msg.origin, msg.target, msg.data, msg.size,
                                   msg.authentication_level);
  if (newMsgPtr) {
    newMsgPtr->request_id = msg.request_id;
    newMsgPtr->type = msg.type;
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DClient::newMsg(
    ESP3DClientType origin, ESP3DClientType target, const uint8_t* data,
    size_t length, ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage* newMsgPtr = newMsg(origin, target, authentication_level);
  if (newMsgPtr) {
    if (!setDataContent(newMsgPtr, data, length)) {
      deleteMsg(newMsgPtr);
      newMsgPtr = nullptr;
    }
  }
  return newMsgPtr;
}

ESP3DMessage* ESP3DClient::newMsg(
    ESP3DClientType origin, ESP3DClientType target,
    ESP3DAuthenticationLevel authentication_level) {
  ESP3DMessage* newMsgPtr = newMsg();
  if (newMsgPtr) {
    newMsgPtr->origin = origin;
    newMsgPtr->target = target;
    newMsgPtr->authentication_level = authentication_level;
  }
  return newMsgPtr;
}

bool ESP3DClient::setDataContent(ESP3DMessage* msg, const uint8_t* data,
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

  // this is centralize if a `\n` is missing at the end of the message, if a
  // client need another end it will handle on it's own
  size_t allocation_needed = length + 1;
  size_t msg_size = length;
  bool missing_endline = false;
  if (msg->type == ESP3DMessageType::unique ||
      msg->type == ESP3DMessageType::tail) {
    if (data[length - 1] != '\n') {
      missing_endline = true;
      allocation_needed++;
    }
  }
  // add some security in case data is called as string so add 1 byte for \0
  msg->data = (uint8_t*)malloc(sizeof(uint8_t) * (allocation_needed));
  if (msg->data) {
    memcpy(msg->data, data, length);
    if (missing_endline) {
      msg->data[msg_size] = '\n';
      msg_size++;
    }
    msg->size = msg_size;
    msg->data[msg_size] =
        '\0';  // add some security in case data is called as string
    return true;
  }
  esp3d_log_e("Out of memory");
  return false;
}
