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
#include <stdio.h>
#include "esp3d_socket_server.h"
#include "esp3d_settings.h"
#include "esp3d_log.h"
#include "esp3d_hal.h"
#include "tasks_def.h"
#include "esp3d_commands.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

ESP3DSocketServer esp3dSocketServer;

#define RX_FLUSH_TIME_OUT 1500  // millisecond timeout
//Use sample code values for the moment
#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 1

#define FREE_SOCKET_HANDLE -1
#define SOCKET_ERROR -1

uint  ESP3DSocketServer::clientsConnected()
{
    uint count = 0;
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
        if (_clients[s].socketId != FREE_SOCKET_HANDLE) {
            count++;
        }
    }
    return count;
}

esp3d_socket_client_info_t * ESP3DSocketServer::getClientInfo(uint index)
{
    if (index<=ESP3D_MAX_SOCKET_CLIENTS) {
        if (_clients[index].socketId !=FREE_SOCKET_HANDLE) {
            return &_clients[index];
        }
    }
    return nullptr;
}

bool ESP3DSocketServer::startSocketServer()
{
    struct sockaddr_storage dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(_port);
    _listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (_listen_socket < 0) {
        esp3d_log_e("Unable to create socket: errno %d", errno);
        return false;
    }

    // Marking the socket as non-blocking
    int flags = fcntl(_listen_socket, F_GETFL);
    if (fcntl(_listen_socket, F_SETFL, flags | O_NONBLOCK) == SOCKET_ERROR) {
        esp3d_log_e("Unable to set socket non blocking: errno %d: %s", errno, strerror(errno));
        close(_listen_socket);
        return false;
    }
    esp3d_log("Socket marked as non blocking");
    //int opt = 1;

    esp3d_log("Socket created");

    int err = bind(_listen_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        esp3d_log_e("Socket unable to bind: errno %d", errno);
        esp3d_log_e("IPPROTO: %d", AF_INET);
        close(_listen_socket);
        return false;
    }
    esp3d_log("Socket bound, port %ld", _port);
    err = listen(_listen_socket, 1);
    if (err != 0) {
        esp3d_log_e("Error occurred during listen: errno %d", errno);
        close(_listen_socket);
        return false;
    }
    return true;
}

void ESP3DSocketServer::process(esp3d_msg_t * msg)
{
    //add to TX queue that will be processed in task
    if (!addTXData(msg)) {
        // delete message as cannot be added to the queue
        Esp3DClient::deleteMsg(msg);
        esp3d_log_e("Failed to add message to tx queue");
    }
}

void ESP3DSocketServer::readSockets()
{
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
            pos[s]=0;
            startTimeout[s]=0;
            if (!_buffer[s]) {
                esp3d_log_e("Missing buffer for socket %d", s);
                return;
            }
        }
    }

    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
        if (_clients[s].socketId != FREE_SOCKET_HANDLE) {
            len = recv(_clients[s].socketId, _data,ESP3D_SOCKET_RX_BUFFER_SIZE, 0);
            if (len < 0) {
                if (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;   // Not an error
                }
                if (errno == ENOTCONN) {
                    esp3d_log("Connection closed");
                    closeSocket(_clients[s].socketId);
                    continue;
                }
                esp3d_log_e("Error occurred during receiving: errno %d on socket %d", errno, s );
                closeSocket(_clients[s].socketId);
            } else {
                _data[len] = 0; // Null-terminate whatever is received and treat it like a string
                esp3d_log("Received %d bytes: %s", len, _data);
                if (len) {
                    //parse data
                    startTimeout[s] = esp3d_hal::millis();
                    for (size_t i = 0; i < len ; i++) {
                        if (pos[s] <ESP3D_SOCKET_RX_BUFFER_SIZE) {
                            _buffer[s][pos[s]]= _data[i];
                            (pos[s])++;
                            _buffer[s][pos[s]]=0;
                        }
                        //if end of char or buffer is full
                        if (isEndChar(_data[i]) || pos[s]==ESP3D_SOCKET_RX_BUFFER_SIZE) {
                            //create message and push
                            if (!pushMsgToRxQueue((const uint8_t*)_buffer[s], pos[s])) {
                                //send error
                                esp3d_log_e("Push Message to rx queue failed");
                            }
                            pos[s]=0;
                        }
                    }
                }
            }
        }
    }
    //if no data during a while then send them
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++)
        if (esp3d_hal::millis()-startTimeout[s]>(RX_FLUSH_TIME_OUT) && pos[s]>0) {
            if (!pushMsgToRxQueue((const uint8_t*)_buffer[s], pos[s])) {
                //send error
                esp3d_log_e("Push Message  %s of size %d to rx queue failed",_buffer[s], pos[s]);
            }
            pos[s]=0;
        }

}

// this task only collecting serial RX data and push thenmm to Rx Queue
static void esp3d_socket_rx_task(void *pvParameter)
{
    (void)pvParameter;
    if (esp3dSocketServer.startSocketServer()) {

        //uint64_t startTimeout = 0; // milliseconds
        while (1) {
            esp3dSocketServer.getClient();
            esp3dSocketServer.readSockets();
            esp3dSocketServer.handle();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    } else {
        esp3d_log_e("Starting socket server failed");
    }
    esp3dSocketServer.end();
}

bool ESP3DSocketServer::sendToSocket(const int sock, const char * data, const size_t len)
{
    int to_write = len;
    //does it need to add time out ?
    while (to_write > 0) {
        int written = send(sock, data + (len - to_write), to_write, 0);
        if (written < 0 && errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK) {
            esp3d_log_e ( "Error occurred during sending %d", errno);
            return false;
        }
        to_write -= written;
    }
    return true;
}

bool ESP3DSocketServer::getClient()
{
    struct sockaddr_storage source_addr;
    socklen_t addr_len = sizeof(source_addr);
    int sock = accept(_listen_socket, (struct sockaddr *)&source_addr, &addr_len);
    if (sock < 0) {
        //esp3d_log_e("Unable to accept connection: errno %d", errno);
        return false;
    }
    int freeIndex = getFreeClientSlot();
    if (freeIndex == SOCKET_ERROR) {
        esp3d_log_e("Unable to get free client slot");
        shutdown(sock, 0);
        close(sock);
        return false;
    }
    //copy informations of the client
    _clients[freeIndex].socketId = sock;
    memcpy(&_clients[freeIndex].source_addr, &source_addr,  sizeof(source_addr));
    // Marking the socket as non-blocking
    int flags = fcntl(_clients[freeIndex].socketId, F_GETFL);
    if (fcntl(_clients[freeIndex].socketId, F_SETFL, flags | O_NONBLOCK) == SOCKET_ERROR) {
        esp3d_log_e("Unable to set socket non blocking: errno %d: %s", errno, strerror(errno));
        closeSocket(_clients[freeIndex].socketId);
        return false;
    }
#if   ESP3D_TFT_LOG
    char addr_str[16];
    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
    esp3d_log("Socket accepted ip address: %s", addr_str);
#endif

    return true;
}

int ESP3DSocketServer::getFreeClientSlot()
{
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
        if (_clients[s].socketId == FREE_SOCKET_HANDLE) {
            return s;
        }
    }
    return SOCKET_ERROR;
}

bool ESP3DSocketServer::closeSocket(int socketId)
{
    if (socketId!=FREE_SOCKET_HANDLE)
        for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
            if (_clients[s].socketId == socketId) {
                shutdown(socketId, 0);
                close(socketId);
                _clients[s].socketId = FREE_SOCKET_HANDLE;
                memset(&_clients[s].source_addr, 0, sizeof(struct sockaddr_storage));
                return true;
            }
        }
    return false;
}

ESP3DSocketServer::ESP3DSocketServer()
{
    _xHandle = NULL;
    _started = false;
    _listen_socket = FREE_SOCKET_HANDLE;
    memset(_clients, 0, sizeof(esp3d_socket_client_info_t) * ESP3D_MAX_SOCKET_CLIENTS);
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
        _clients[s].socketId = FREE_SOCKET_HANDLE;
    }
    _data = NULL;
    _buffer = NULL;
}
ESP3DSocketServer::~ESP3DSocketServer()
{
    end();
}

bool ESP3DSocketServer::isEndChar(uint8_t ch)
{
    return ((char)ch == '\n' || (char)ch=='\r');
}

bool ESP3DSocketServer::begin()
{
    end();
    if (esp3d_state_on != (esp3d_state_t)esp3dTFTsettings.readByte(esp3d_socket_on)) {
        esp3d_log("Telnet is not enabled");
        // return true because no error but _started is false
        return true;
    }
    //Initialize client buffer
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

    //Read port
    _port = esp3dTFTsettings.readUint32(esp3d_socket_port);

    _data = (char *) malloc(ESP3D_SOCKET_RX_BUFFER_SIZE+1);
    if (_data == NULL) {
        esp3d_log_e("Memory allocation failed");
        _started = false;
        return false;
    }
    _buffer = (char **) malloc(ESP3D_MAX_SOCKET_CLIENTS*sizeof(char *));
    if (_buffer == NULL) {
        esp3d_log_e("Memory allocation failed");
        _started = false;
        return false;
    }

    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
        _buffer[s] = (char *) malloc(ESP3D_SOCKET_RX_BUFFER_SIZE+1);
        if (_buffer[s] == NULL) {
            esp3d_log_e("Memory allocation failed");
            _started = false;
            return false;
        }
    }

    BaseType_t res = xTaskCreatePinnedToCore(esp3d_socket_rx_task, "esp3d_socket_rx_task", ESP3D_SOCKET_TASK_SIZE, NULL, ESP3D_SOCKET_TASK_PRIORITY, &_xHandle, ESP3D_SOCKET_TASK_CORE);

    if (res == pdPASS && _xHandle) {
        esp3d_log("Created Socket Task");
        esp3d_log("Socket Server started port %ld", _port);
        _started = true;
        return true;
    } else {
        esp3d_log_e("Serial Task creation failed");
        _started = false;
        return false;
    }
}

bool ESP3DSocketServer::pushMsgToRxQueue(const uint8_t *msg, size_t size)
{
    esp3d_log("Pushing `%s` %d", msg, size);
    esp3d_msg_t *newMsgPtr = newMsg();
    if (newMsgPtr) {
        if (Esp3DClient::setDataContent(newMsgPtr, msg, size)) {
#if     ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
            newMsgPtr->authentication_level = ESP3D_LEVEL_ADMIN;
#endif // ESP3D_DISABLE_SERIAL_AUTHENTICATION
            newMsgPtr->origin = TELNET_CLIENT;
            newMsgPtr->target= SERIAL_CLIENT;
            newMsgPtr->type = msg_unique;
            if (!addRXData(newMsgPtr)) {
                // delete message as cannot be added to the queue
                Esp3DClient::deleteMsg(newMsgPtr);
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


void ESP3DSocketServer::handle()
{
    if (_started) {
        if (getRxMsgsCount() > 0) {
            esp3d_msg_t *msg = popRx();
            if (msg) {
                esp3dCommands.process(msg);
            }
        }
        if (getTxMsgsCount() > 0) {
            esp3d_msg_t *msg = popTx();
            if (msg) {
                for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
                    if (_clients[s].socketId != FREE_SOCKET_HANDLE) {
                        sendToSocket(_clients[s].socketId, (const char *)msg->data, msg->size);
                    }
                }
                deleteMsg(msg);
            }
        }
    }
}

void ESP3DSocketServer::flush()
{
    uint8_t loopCount = 10;
    while (loopCount && getTxMsgsCount() > 0) {
        // esp3d_log("flushing Tx messages");
        loopCount--;
        handle();
    }
}

void ESP3DSocketServer::end()
{
    if (_started) {
        flush();
        _started = false;
        esp3d_log("Clearing queue Rx messages");
        clearRxQueue();
        esp3d_log("Clearing queue Tx messages");
        clearTxQueue();
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (pthread_mutex_destroy(&_tx_mutex) != 0) {
            esp3d_log_w("Mutex destruction for tx failed");
        }
        if (pthread_mutex_destroy(&_rx_mutex) != 0) {
            esp3d_log_w("Mutex destruction for rx failed");
        }
        esp3d_log("Stop telnet server");
        // TODO
        _port = 0;
    }
    if (_xHandle) {
        vTaskDelete(_xHandle);
        _xHandle = NULL;
    }
    if (_data) {
        free(_data);
        _data = NULL;
    }
    if (_buffer) {
        for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
            if(_buffer[s]) {
                free(_buffer[s]);
                _buffer[s] = NULL;
            }
        }
        free(_buffer);
        _buffer = NULL;
    }
    _listen_socket = FREE_SOCKET_HANDLE;
    closeAllClients();
}

void ESP3DSocketServer::closeAllClients()
{
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
        closeSocket(_clients[s].socketId);
    }
}
