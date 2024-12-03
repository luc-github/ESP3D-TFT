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
#include "esp3d_socket_server.h"

#include <lwip/netdb.h>
#include <stdio.h>

#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_version.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "tasks_def.h"

ESP3DSocketServer esp3dSocketServer;

#define RX_FLUSH_TIME_OUT 1500  // millisecond timeout
// Use sample code values for the moment
#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 1

#if ESP3D_AUTHENTICATION_FEATURE
#define WELCOME_MSG                          \
  "Welcome to ESP3D-TFT V" ESP3D_TFT_VERSION \
  ", please enter a command with credentials.\r\n"
#define ERROR_MSG "Authentication error, rejected."
#else
#define WELCOME_MSG "Welcome to ESP3D-TFT V" ESP3D_TFT_VERSION ".\r\n"
#endif

#define FREE_SOCKET_HANDLE -1
#define SOCKET_ERROR -1

uint ESP3DSocketServer::clientsConnected() {
  uint count = 0;
  if (_started) {
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
      if (_clients[s].socket_id != FREE_SOCKET_HANDLE) {
        count++;
      }
    }
  }
  return count;
}

void ESP3DSocketServer::closeMainSocket() {
  shutdown(_listen_socket, 0);
  close(_listen_socket);
  _listen_socket = FREE_SOCKET_HANDLE;
}

ESP3DSocketInfos *ESP3DSocketServer::getClientInfos(uint index) {
  if (_started) {
    if (index <= ESP3D_MAX_SOCKET_CLIENTS) {
      if (_clients[index].socket_id != FREE_SOCKET_HANDLE) {
        return &_clients[index];
      }
    }
  }
  return nullptr;
}

bool ESP3DSocketServer::startSocketServer() {
  _isRunning = false;
  struct sockaddr_storage dest_addr;
  struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
  dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
  dest_addr_ip4->sin_family = AF_INET;
  dest_addr_ip4->sin_port = htons(_port);
  if (_listen_socket != FREE_SOCKET_HANDLE) {
    closeMainSocket();
  }
  _listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (_listen_socket < 0) {
    esp3d_log_e("Unable to create socket: errno %d", errno);
    return false;
  }
  // https://github.com/espressif/esp-idf/issues/6394#issuecomment-762218598
  // workarround for ip already in use when it was already closed
  /*BaseType_t xTrueValue = pdTRUE;
  setsockopt(_listen_socket, SOL_SOCKET, SO_REUSEADDR, (void *)&xTrueValue,
             sizeof(xTrueValue));*/

  // Marking the socket as non-blocking
  int flags = fcntl(_listen_socket, F_GETFL);
  if (fcntl(_listen_socket, F_SETFL, flags | O_NONBLOCK) == SOCKET_ERROR) {
    esp3d_log_e("Unable to set socket non blocking: errno %d: %s", errno,
                strerror(errno));
    closeMainSocket();
    return false;
  }
  esp3d_log("Socket marked as non blocking");

  esp3d_log("Socket created");

  int err =
      bind(_listen_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
  if (err != 0) {
    esp3d_log_e("Socket unable to bind: errno %d, %s", errno, strerror(errno));
    esp3d_log_e("IPPROTO: %d", AF_INET);
    closeMainSocket();
    return false;
  }
  esp3d_log("Socket bound, port %ld", _port);
  err = listen(_listen_socket, 1);
  if (err != 0) {
    esp3d_log_e("Error occurred during listen: errno %d, %s", errno,
                strerror(errno));
    closeMainSocket();
    return false;
  }
  _isRunning = true;
  return true;
}

void ESP3DSocketServer::process(ESP3DMessage *msg) {
  if (!addTxData(msg)) {
    flush();
    if (!addTxData(msg)) {
      esp3d_log_e("Cannot add msg to client queue");
      deleteMsg(msg);
    }
  }
}

void ESP3DSocketServer::readSockets() {
  int len;
  static uint64_t startTimeout[ESP3D_MAX_SOCKET_CLIENTS];
  static bool initArrays = false;
  if (!_started || !_data || !_buffer) {
    return;
  }
  static size_t pos[ESP3D_MAX_SOCKET_CLIENTS];
  if (!initArrays) {
    initArrays = true;
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
      pos[s] = 0;
      startTimeout[s] = 0;
      if (!_buffer[s]) {
        esp3d_log_e("Missing buffer for socket %d", s);
        return;
      }
    }
  }

  for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
    if (_clients[s].socket_id != FREE_SOCKET_HANDLE) {
      len = recv(_clients[s].socket_id, _data, ESP3D_SOCKET_RX_BUFFER_SIZE, 0);
      if (len < 0) {
        if (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK) {
          continue;  // Not an error
        }
        if (errno == ENOTCONN) {
          esp3d_log("Connection closed");
          closeSocket(_clients[s].socket_id);
          continue;
        }
        esp3d_log_e("Error occurred during receiving: errno %d on socket %d",
                    errno, s);
        closeSocket(_clients[s].socket_id);
      } else {
        _data[len] = 0;  // Null-terminate whatever is received and treat it
                         // like a string
        esp3d_log("Received %d bytes: %s", len, _data);
        if (len) {
          // parse data
          startTimeout[s] = esp3d_hal::millis();
          for (size_t i = 0; i < len; i++) {
            if (pos[s] < ESP3D_SOCKET_RX_BUFFER_SIZE) {
              _buffer[s][pos[s]] = _data[i];
              (pos[s])++;
              _buffer[s][pos[s]] = 0;
            }
            // if end of char or buffer is full
            if (isEndChar(_data[i]) || pos[s] == ESP3D_SOCKET_RX_BUFFER_SIZE) {
              // create message and push
              if (!pushMsgToRxQueue(s, (const uint8_t *)_buffer[s], pos[s])) {
                // send error
                esp3d_log_e("Push Message to rx queue failed");
              }
              pos[s] = 0;
            }
          }
        }
      }
    }
  }
  // if no data during a while then send them
  for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++)
    if (esp3d_hal::millis() - startTimeout[s] > (RX_FLUSH_TIME_OUT) &&
        pos[s] > 0) {
      if (!pushMsgToRxQueue(s, (const uint8_t *)_buffer[s], pos[s])) {
        // send error
        esp3d_log_e("Push Message  %s of size %d to rx queue failed",
                    _buffer[s], pos[s]);
      }
      pos[s] = 0;
    }
}

// this task only collecting serial RX data and push thenmm to Rx Queue
static void esp3d_socket_rx_task(void *pvParameter) {
  (void)pvParameter;
  bool res = esp3dSocketServer.startSocketServer();

  if (res) {
    // uint64_t startTimeout = 0; // milliseconds
    while (esp3dSocketServer.isRunning()) {
      esp3dSocketServer.getClient();
      esp3dSocketServer.readSockets();
      esp3dSocketServer.handle();
      esp3d_hal::wait(10);
    }

  } else {
    esp3d_log_e("Starting socket server failed");
    esp3dSocketServer.end();
  }
  esp3dSocketServer.resetTaskHandle();
  vTaskDelete(NULL);
}

bool ESP3DSocketServer::sendToSocket(const int sock, const char *data,
                                     const size_t len) {
  int to_write = len;
  // does it need to add time out ?
  while (to_write > 0) {
    int written = send(sock, data + (len - to_write), to_write, 0);
    if (written < 0 && errno != EINPROGRESS && errno != EAGAIN &&
        errno != EWOULDBLOCK) {
      esp3d_log_e("Error occurred during sending %d", errno);
      return false;
    }
    to_write -= written;
  }
  return true;
}

bool ESP3DSocketServer::getClient() {
  struct sockaddr_storage source_addr;
  socklen_t addr_len = sizeof(source_addr);
  int sock = accept(_listen_socket, (struct sockaddr *)&source_addr, &addr_len);
  if (sock < 0) {
    // esp3d_log_e("Unable to accept connection: errno %d", errno);
    return false;
  }
  int freeIndex = getFreeClientSlot();
  if (freeIndex == SOCKET_ERROR) {
    esp3d_log_e("Unable to get free client slot");
    shutdown(sock, 0);
    close(sock);
    return false;
  }
#if ESP3D_AUTHENTICATION_FEATURE
  memset(_clients[freeIndex].session_id, 0,
         sizeof(_clients[freeIndex].session_id));
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
  // copy informations of the client
  _clients[freeIndex].socket_id = sock;
  memcpy(&_clients[freeIndex].source_addr, &source_addr, sizeof(source_addr));
  // Marking the socket as non-blocking
  int flags = fcntl(_clients[freeIndex].socket_id, F_GETFL);
  if (fcntl(_clients[freeIndex].socket_id, F_SETFL, flags | O_NONBLOCK) ==
      SOCKET_ERROR) {
    esp3d_log_e("Unable to set socket non blocking: errno %d: %s", errno,
                strerror(errno));
    closeSocket(_clients[freeIndex].socket_id);
    return false;
  }
#if ESP3D_TFT_LOG
  char addr_str[16];
  inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str,
              sizeof(addr_str) - 1);
  esp3d_log("Socket accepted ip address: %s", addr_str);
#endif
#if !DISABLE_TELNET_WELCOME_MESSAGE
  sendToSocket(sock, WELCOME_MSG, strlen(WELCOME_MSG));
#endif  // DISABLE_TELNET_WELCOME_MESSAGE
  return true;
}

int ESP3DSocketServer::getFreeClientSlot() {
  for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
    if (_clients[s].socket_id == FREE_SOCKET_HANDLE) {
      return s;
    }
  }
  return SOCKET_ERROR;
}

bool ESP3DSocketServer::closeSocket(int socketId) {
  if (socketId != FREE_SOCKET_HANDLE)

    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
      if (_clients[s].socket_id == socketId) {
#if ESP3D_AUTHENTICATION_FEATURE
        esp3dAuthenthicationService.clearSession(_clients[s].session_id);
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
        shutdown(socketId, 0);
        close(socketId);
        _clients[s].socket_id = FREE_SOCKET_HANDLE;
        memset(&_clients[s].source_addr, 0, sizeof(struct sockaddr_storage));
        return true;
      }
    }
  return false;
}

ESP3DSocketServer::ESP3DSocketServer() {
  _xHandle = NULL;
  _started = false;
  _listen_socket = FREE_SOCKET_HANDLE;
  memset(_clients, 0, sizeof(ESP3DSocketInfos) * ESP3D_MAX_SOCKET_CLIENTS);
  for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
    _clients[s].socket_id = FREE_SOCKET_HANDLE;
  }
  _data = NULL;
  _buffer = NULL;
}
ESP3DSocketServer::~ESP3DSocketServer() { end(); }

bool ESP3DSocketServer::isEndChar(uint8_t ch) {
  return ((char)ch == '\n' || (char)ch == '\r');
}

bool ESP3DSocketServer::begin() {
  end();
  if (ESP3DState::on != (ESP3DState)esp3dTftsettings.readByte(
                            ESP3DSettingIndex::esp3d_socket_on)) {
    esp3d_log("Telnet is not enabled");
    // return true because no error but _started is false
    return true;
  }
  if (_xHandle) {
    esp3d_log_e("ESP3DSocketServer already has task, invalide state");
    return false;
  }
  // Initialize client buffer
  if (pthread_mutex_init(&_rx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for rx failed");
    return false;
  }
  setRxMutex(&_rx_mutex);

  if (pthread_mutex_init(&_tx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for tx failed");
    return false;
  }
  setTxMutex(&_tx_mutex);

  // Read port
  _port = esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_socket_port);

  _data = (char *)malloc(ESP3D_SOCKET_RX_BUFFER_SIZE + 1);
  if (_data == NULL) {
    esp3d_log_e("Memory allocation failed");
    _started = false;
    return false;
  }
  _buffer = (char **)malloc(ESP3D_MAX_SOCKET_CLIENTS * sizeof(char *));
  if (_buffer == NULL) {
    esp3d_log_e("Memory allocation failed");
    _started = false;
    return false;
  }

  for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
    _buffer[s] = (char *)malloc(ESP3D_SOCKET_RX_BUFFER_SIZE + 1);
    if (_buffer[s] == NULL) {
      esp3d_log_e("Memory allocation failed");
      _started = false;
      return false;
    }
  }

  BaseType_t res = xTaskCreatePinnedToCore(
      esp3d_socket_rx_task, "esp3d_socket_rx_task", ESP3D_SOCKET_TASK_SIZE,
      NULL, ESP3D_SOCKET_TASK_PRIORITY, &_xHandle, ESP3D_SOCKET_TASK_CORE);

  if (res == pdPASS && _xHandle) {
    esp3d_log("Created Socket Task");
    esp3d_log("Socket Server started port %ld", _port);
    _started = true;
    return true;
  } else {
    esp3d_log_e("Socket Task creation failed");
    _started = false;
    return false;
  }
}

bool ESP3DSocketServer::pushMsgToRxQueue(uint index, const uint8_t *msg,
                                         size_t size) {
  ESP3DSocketInfos *client = getClientInfos(index);
  if (client == NULL) {
    esp3d_log_e("No client");
    return false;
  }
  ESP3DAuthenticationLevel authentication_level =
      ESP3DAuthenticationLevel::guest;
#if ESP3D_AUTHENTICATION_FEATURE
  esp3d_log("Check authentication level");
  if (strlen(client->session_id) == 0) {
    esp3d_log("No sessionId");
    std::string str =
        esp3dCommands.get_param((const char *)msg, size, 0, "pwd=");
    authentication_level =
        esp3dAuthenthicationService.getAuthenticatedLevel(str.c_str());
    esp3d_log("Authentication Level = %d",
              static_cast<uint8_t>(authentication_level));
    if (authentication_level == ESP3DAuthenticationLevel::guest) {
      esp3d_log("Authentication Level = GUEST, for %s", (const char *)msg);
      sendToSocket(client->socket_id, ERROR_MSG, strlen(ERROR_MSG));
      return false;
    }
    // 1 -  create  session id
    // 2 -  add session id to client info
    strcpy(client->session_id, esp3dAuthenthicationService.create_session_id(
                                   client->source_addr, client->socket_id));
    // 3 -  add session id to _sessions list
    if (!esp3dAuthenthicationService.createRecord(
            client->session_id, client->socket_id, authentication_level,
            ESP3DClientType::telnet)) {
      esp3d_log("Authentication error, rejected.");
      sendToSocket(client->socket_id, ERROR_MSG, strlen(ERROR_MSG));
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
  esp3d_log("Pushing `%s` %d", msg, size);
  ESP3DMessage *newMsgPtr = newMsg();
  if (newMsgPtr) {
    if (ESP3DClient::setDataContent(newMsgPtr, msg, size)) {
      newMsgPtr->origin = ESP3DClientType::telnet;
      newMsgPtr->authentication_level = authentication_level;
      newMsgPtr->target = ESP3DClientType::stream;
      newMsgPtr->type = ESP3DMessageType::unique;
      newMsgPtr->request_id.id = client->socket_id;
      if (!addRxData(newMsgPtr)) {
        // delete message as cannot be added to the queue
        ESP3DClient::deleteMsg(newMsgPtr);
        esp3d_log_e("Failed to add message to rx queue");
        return false;
      }
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

void ESP3DSocketServer::handle() {
  if (_started) {
    if (getRxMsgsCount() > 0) {
      ESP3DMessage *msg = popRx();
      if (msg) {
        esp3dCommands.process(msg);
      }
    }
    if (getTxMsgsCount() > 0) {
      ESP3DMessage *msg = popTx();
      if (msg) {
        for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
          if (_clients[s].socket_id != FREE_SOCKET_HANDLE) {
            if (msg->request_id.id == 0 ||
                msg->request_id.id == _clients[s].socket_id) {
              sendToSocket(_clients[s].socket_id, (const char *)msg->data,
                           msg->size);
            }
          }
        }
        deleteMsg(msg);
      }
    }
  }
}

void ESP3DSocketServer::flush() {
  uint8_t loopCount = 10;
  while (loopCount && getTxMsgsCount() > 0) {
    esp3d_log("flushing Tx messages");
    loopCount--;
    handle();
  }
}

void ESP3DSocketServer::end() {
  if (_started) {
    esp3d_log("End socket server");
    _isRunning = false;
    esp3d_hal::wait(500);
    flush();
    closeAllClients();
    _started = false;
    esp3d_log("Clearing queue Rx messages");
    clearRxQueue();
    esp3d_log("Clearing queue Tx messages");
    clearTxQueue();
    esp3d_hal::wait(500);
    if (pthread_mutex_destroy(&_tx_mutex) != 0) {
      esp3d_log_w("Mutex destruction for tx failed");
    }
    if (pthread_mutex_destroy(&_rx_mutex) != 0) {
      esp3d_log_w("Mutex destruction for rx failed");
    }
    esp3d_log("Stop telnet server");
    closeMainSocket();
    _port = 0;
  }
  // Sanity checks that need to be done even server is not started
  // No need to kill task since server auto delete task when loop is not running
  /*if (_xHandle) {
    vTaskDelete(_xHandle);
    _xHandle = NULL;

  }*/
  if (_data) {
    free(_data);
    _data = NULL;
    esp3d_log("Socket server data cleared");
  }
  if (_buffer) {
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
      if (_buffer[s]) {
        free(_buffer[s]);
        _buffer[s] = NULL;
      }
    }
    free(_buffer);
    _buffer = NULL;
    esp3d_log("Socket server buffer cleared");
  }
}

void ESP3DSocketServer::closeAllClients() {
  esp3d_log("Socket server closing all clients");
  for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
    closeSocket(_clients[s].socket_id);
  }
}
