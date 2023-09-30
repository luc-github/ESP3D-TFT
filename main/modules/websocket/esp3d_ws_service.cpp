/*
  esp3d_http_service
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
#include "esp3d_ws_service.h"

#include <esp_system.h>
#include <stdio.h>

#include "esp3d_commands.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_version.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "http/esp3d_http_service.h"
#include "tasks_def.h"

#if ESP3D_WS_SERVICE_FEATURE
ESP3DWsService esp3dWsDataService;
#endif  // ESP3D_WS_SERVICE_FEATURE

#if ESP3D_AUTHENTICATION_FEATURE
#define WELCOME_MSG                          \
  "Welcome to ESP3D-TFT V" ESP3D_TFT_VERSION \
  ", please enter a command with credentials.\n"
#define ERROR_MSG "Authentication error, rejected.\n"
#else
#define WELCOME_MSG "Welcome to ESP3D-TFT V" ESP3D_TFT_VERSION ".\n"
#endif

#define NO_FREE_SOCKET_HANDLE -1
#define FREE_SOCKET_HANDLE -1
#define SOCKET_ERROR -1

int ESP3DWsService::getFreeClientIndex() {
  for (uint i = 0; i < _max_clients; i++) {
    if (_clients[i].socket_id == FREE_SOCKET_HANDLE) {
      return i;
    }
  }
  return NO_FREE_SOCKET_HANDLE;
}
uint ESP3DWsService::clientsConnected() {
  uint count = 0;
  for (uint i = 0; i < _max_clients; i++) {
    if (_clients[i].socket_id != FREE_SOCKET_HANDLE) {
      count++;
      esp3d_log("%d socket:%d", count, _clients[i].socket_id);
    }
  }
  return count;
}

ESP3DWebSocketInfos *ESP3DWsService::getClientInfosFromSocketId(int socketId) {
  for (uint index = 0; index < _max_clients; index++) {
    if (_clients[index].socket_id == socketId) {
      return &_clients[index];
      ;
    }
  }
  return nullptr;
}

ESP3DWebSocketInfos *ESP3DWsService::getClientInfos(uint index) {
  if (index < _max_clients) {
    if (_clients[index].socket_id != FREE_SOCKET_HANDLE) {
      return &_clients[index];
    }
  }
  return nullptr;
}

bool ESP3DWsService::closeClient(int socketId) {
  if (!_started || !_server) {
    esp3d_log_e("Not started or not server");
    return false;
  }
  httpd_sess_trigger_close(_server, socketId);
  onClose(socketId);
  return true;
}

void ESP3DWsService::closeClients() {
  if (!_started || !_server) {
    esp3d_log_e("Not started or not server");
    return;
  }
  for (uint i = 0; i < _max_clients; i++) {
    if (_clients[i].socket_id != FREE_SOCKET_HANDLE) {
      esp3d_log("Closing %d ", _clients[i].socket_id);
      closeClient(_clients[i].socket_id);
    }
  }
}

bool ESP3DWsService::addClient(int socketid) {
  int freeIndex = getFreeClientIndex();
  if (freeIndex == NO_FREE_SOCKET_HANDLE) {
    esp3d_log_e("No free slot for new connection, rejects connection");
    return false;
  }
  _clients[freeIndex].socket_id = socketid;
  _clients[freeIndex].buf_position = 0;
  esp3d_log("Added connection %d, on slot %d", socketid, freeIndex);

  struct sockaddr_in6 saddr;  // esp_http_server uses IPv6 addressing
  socklen_t saddr_len = sizeof(saddr);
  if (getpeername(socketid, (struct sockaddr *)&saddr, &saddr_len) >= 0) {
    static char address_str[40];
    inet_ntop(AF_INET, &saddr.sin6_addr.un.u32_addr[3], address_str,
              sizeof(address_str));
    esp3d_log("client IP is %s", address_str);
    ((struct sockaddr_in *)(&_clients[freeIndex].source_addr))
        ->sin_addr.s_addr = saddr.sin6_addr.un.u32_addr[3];
  } else {
    esp3d_log_e("Failed to get address for new connection");
  }
#if ESP3D_AUTHENTICATION_FEATURE
  memset(_clients[freeIndex].session_id, 0,
         sizeof(_clients[freeIndex].session_id));
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
  return true;
}

ESP3DWsService::ESP3DWsService() {
  _started = false;
  _server = nullptr;
  _max_clients = 0;
  _clients = nullptr;
  _type = esp3dSocketType::unknown;
}

void ESP3DWsService::end() {
  if (!_started) {
    return;
  }
  esp3d_log("Stop Ws Service %d", static_cast<uint8_t>(_type));
  closeClients();
  _server = nullptr;
  _started = false;
  if (_clients) {
    for (uint s = 0; s < _max_clients; s++) {
      if (_clients[s].buffer) {
        free(_clients[s].buffer);
      }
    }
    free(_clients);
    _clients = nullptr;
  }
  _max_clients = 0;
  _type = esp3dSocketType::unknown;
}

ESP3DWsService::~ESP3DWsService() { end(); }

bool ESP3DWsService::begin(ESP3DWebSocketConfig *config) {
  esp3d_log("Starting Ws Service");
  end();
  _server = config->server_handle;
  _type = config->type;
  if (_server == nullptr) {
    esp3d_log_e("No valid server handle");
    return false;
  }
  _max_clients = config->max_clients;
  if (_max_clients == 0) {
    esp3d_log_e("max_clients cannot be 0");
    return false;
  }
  _clients =
      (ESP3DWebSocketInfos *)malloc(_max_clients * sizeof(ESP3DWebSocketInfos));
  if (_clients == NULL) {
    esp3d_log_e("Memory allocation failed");
    _started = false;
    return false;
  }

  for (uint s = 0; s < _max_clients; s++) {
    _clients[s].buffer = (char *)malloc(ESP3D_WS_RX_BUFFER_SIZE + 1);
    if (_clients[s].buffer == NULL) {
      esp3d_log_e("Memory allocation failed");
      _started = false;
      return false;
    }
    _clients[s].buf_position = 0;
    _clients[s].socket_id = FREE_SOCKET_HANDLE;
  }
  _started = true;
  if (_started) {
    esp3d_log("Ws Service Started type:%d", static_cast<uint8_t>(_type));
  }
  return _started;
}

void ESP3DWsService::handle() {}

esp_err_t ESP3DWsService::onOpen(httpd_req_t *req) {
  int currentFd = httpd_req_to_sockfd(req);
  if (!addClient(currentFd)) {
    esp3d_log_e("Failed to add client");
    httpd_sess_trigger_close(_server, currentFd);
  }
  pushMsgTxt(currentFd, WELCOME_MSG);
  return ESP_OK;
}

bool ESP3DWsService::isEndChar(uint8_t ch) {
  return ((char)ch == '\n' || (char)ch == '\r');
}

esp_err_t ESP3DWsService::onMessage(httpd_req_t *req) {
  httpd_ws_frame_t ws_pkt;
  uint8_t *buf = NULL;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  int currentFd = httpd_req_to_sockfd(req);
  esp3d_log("Message from %d", currentFd);
  ESP3DWebSocketInfos *client = getClientInfosFromSocketId(currentFd);
  if (client == NULL) {
    esp3d_log_e("Unregistered client");
    return ESP_FAIL;
  }
  /* Set max_len = 0 to get the frame len */
  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
  if (ret != ESP_OK) {
    esp3d_log_e("httpd_ws_recv_frame failed to get frame len with %s",
                esp_err_to_name(ret));
    return ret;
  }
  // esp3d_log("frame len is %d", ws_pkt.len);
  if (ws_pkt.len) {
    /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
    buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
    if (buf == NULL) {
      esp3d_log_e("Failed to calloc memory for buf");
      return ESP_ERR_NO_MEM;
    }
    ws_pkt.payload = buf;
    /* Set max_len = ws_pkt.len to get the frame payload */
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK) {
      esp3d_log_e("httpd_ws_recv_frame failed with %s", esp_err_to_name(ret));
      free(buf);
      return ret;
    }
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
      buf[ws_pkt.len] = 0;
      esp3d_log("Got packet with message: %s", buf);
      // process payload and dispatch message is \n \re
      for (uint i = 0; i < ws_pkt.len; i++) {
        if (client->buf_position < ESP3D_WS_RX_BUFFER_SIZE) {
          client->buffer[client->buf_position] = buf[i];
          (client->buf_position)++;
          client->buffer[client->buf_position] = 0;
        }
        // if end of char or buffer is full
        if (isEndChar(buf[i]) ||
            client->buf_position == ESP3D_SOCKET_RX_BUFFER_SIZE) {
          if (!pushMsgToRxQueue(client->socket_id,
                                (const uint8_t *)client->buffer,
                                client->buf_position)) {
            // send error
            esp3d_log_e("Push Message to rx queue failed");
          }
          client->buf_position = 0;
        }
      }
    } else if (ws_pkt.type == HTTPD_WS_TYPE_BINARY) {
      esp3d_log("Got packet with %d bytes", ws_pkt.len);
      // TODO process payload / file upload ? =>TBD
    } else {
      esp3d_log_e("Unknown frame type %d", ws_pkt.type);
    }
  } else {
    esp3d_log_e("Empty frame type %d", ws_pkt.type);
  }
  free(buf);
  return ESP_OK;
}

bool ESP3DWsService::pushMsgToRxQueue(int socketId, const uint8_t *msg,
                                      size_t size) {
  esp3d_log("Pushing `%s` %d", msg, size);
  ESP3DWebSocketInfos *client = getClientInfosFromSocketId(socketId);
  if (client == NULL) {
    esp3d_log_e("No client");
    return false;
  }
  ESP3DAuthenticationLevel authentication_level =
      ESP3DAuthenticationLevel::guest;
#if ESP3D_AUTHENTICATION_FEATURE
  esp3d_log("Check authentication level");
  if (strlen(client->session_id) == 0) {
    esp3d_log("No session_id");
    std::string str =
        esp3dCommands.get_param((const char *)msg, size, 0, "pwd=");
    authentication_level =
        esp3dAuthenthicationService.getAuthenticatedLevel(str.c_str());
    esp3d_log("Authentication Level = %d",
              static_cast<uint8_t>(authentication_level));
    if (authentication_level == ESP3DAuthenticationLevel::guest) {
      esp3d_log("Authentication Level = GUEST, for %s", (const char *)msg);
      pushMsgTxt(client->socket_id, ERROR_MSG);
      return false;
    }
    // 1 -  create  session id
    // 2 -  add session id to client info
    strcpy(client->session_id, esp3dAuthenthicationService.create_session_id(
                                   client->source_addr, client->socket_id));
    // 3 -  add session id to _sessions list
    if (!esp3dAuthenthicationService.createRecord(
            client->session_id, client->socket_id, authentication_level,
            ESP3DClientType::websocket)) {
      esp3d_log("Authentication error, rejected.");
      pushMsgTxt(client->socket_id, ERROR_MSG);
      return false;
    }
  } else {
    esp3d_log("SessionId is %s", client->session_id);
    // No need to check time out as session is deleted on close
    ESP3DAuthenticationRecord *rec =
        esp3dAuthenthicationService.getRecord(client->session_id);
    if (rec != NULL) {
      authentication_level = rec->level;
    } else {
      esp3d_log_e("No client record for authentication level");
      return false;
    }
  }
#else
  authentication_level = ESP3DAuthenticationLevel::admin;
#endif  // ESP3D_AUTHENTICATION_FEATURE

  ESP3DMessage *newMsgPtr = ESP3DClient::newMsg();
  if (newMsgPtr) {
    if (ESP3DClient::setDataContent(newMsgPtr, msg, size)) {
      newMsgPtr->authentication_level = authentication_level;
      newMsgPtr->origin = ESP3DClientType::websocket;
      newMsgPtr->target = ESP3DClientType::stream;
      newMsgPtr->type = ESP3DMessageType::unique;
      newMsgPtr->request_id.id = socketId;
      if (esp3dCommands.is_esp_command((uint8_t *)msg, size)) {
        newMsgPtr->target = ESP3DClientType::command;
      }
      esp3dCommands.process(newMsgPtr);
    } else {
      // delete message as cannot be added partially filled to the queue
      free(newMsgPtr);
      esp3d_log_e("Message creation failed");
      return false;
    }
  } else {
    esp3d_log_e("Out of memory!");
    return false;
  }
  return true;
}

esp_err_t ESP3DWsService::onClose(int fd) {
  for (uint i = 0; i < _max_clients; i++) {
    if (_clients[i].socket_id == fd) {
#if ESP3D_AUTHENTICATION_FEATURE
      esp3dAuthenthicationService.clearSession(_clients[i].session_id);
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
      _clients[i].buf_position = 0;
      _clients[i].socket_id = FREE_SOCKET_HANDLE;
      esp3d_log("onClose %d succeed", fd);
      return ESP_OK;
    }
  }
  // esp3d_log_w("onClose %d failed", fd);
  return ESP_FAIL;
}

void ESP3DWsService::process(ESP3DMessage *msg) {
  esp3d_log("Processing message");
  // Broadcast ?
  if (msg->request_id.id == 0) {
    BroadcastTxt(msg->data, msg->size);
  } else {
    pushMsgTxt(msg->request_id.id, msg->data, msg->size);
  }
  ESP3DClient::deleteMsg(msg);
}

esp_err_t ESP3DWsService::http_handler(httpd_req_t *req) {
  if (!_started) {
    return ESP_FAIL;
  }
  if (req->method == HTTP_GET) {
    esp3d_log("WS Handshake done");
    onOpen(req);
    return ESP_OK;
  }
  return onMessage(req);
}

esp_err_t ESP3DWsService::pushMsgTxt(int fd, const char *msg) {
  return pushMsgTxt(fd, (uint8_t *)msg, strlen(msg));
}

esp_err_t ESP3DWsService::pushMsgTxt(int fd, uint8_t *msg, size_t len) {
  if (!_started || fd == -1) {
    return ESP_FAIL;
  }
  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  ws_pkt.payload = msg;
  ws_pkt.len = len;
  esp_err_t res = httpd_ws_send_frame_async(_server, fd, &ws_pkt);
  if (res != ESP_OK) {
    esp3d_log_e("httpd_ws_send_frame failed with %s", esp_err_to_name(res));
  }
  return res;
}

esp_err_t ESP3DWsService::pushMsgBin(int fd, uint8_t *msg, size_t len) {
  if (!_started) {
    return ESP_FAIL;
  }
  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_BINARY;
  ws_pkt.payload = msg;
  ws_pkt.len = len;
  esp_err_t res = httpd_ws_send_frame_async(_server, fd, &ws_pkt);
  if (res != ESP_OK) {
    esp3d_log_e("httpd_ws_send_frame failed with %s", esp_err_to_name(res));
  }
  return res;
}

esp_err_t ESP3DWsService::BroadcastTxt(const char *msg, int ignore) {
  return BroadcastTxt((uint8_t *)msg, strlen(msg), ignore);
}

esp_err_t ESP3DWsService::BroadcastTxt(uint8_t *msg, size_t len, int ignore) {
  if (!_started) {
    return ESP_FAIL;
  }
  for (uint i = 0; i < _max_clients; i++) {
    if (!(_clients[i].socket_id == ignore || _clients[i].socket_id == -1)) {
      esp3d_log("Broadcast to socket %d", _clients[i].socket_id);
      pushMsgTxt(_clients[i].socket_id, msg, len);
    }
  }
  return ESP_OK;
}

esp_err_t ESP3DWsService::BroadcastBin(uint8_t *msg, size_t len, int ignore) {
  if (!_started) {
    return ESP_FAIL;
  }
  for (uint i = 0; i < _max_clients; i++) {
    if (!(_clients[i].socket_id == ignore || _clients[i].socket_id == -1)) {
      esp3d_log("Broadcast to socket %d", _clients[i].socket_id);
      pushMsgBin(_clients[i].socket_id, msg, len);
    }
  }
  return ESP_OK;
}
