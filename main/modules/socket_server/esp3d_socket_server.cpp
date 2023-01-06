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

ESP3DSocketServer socketServer;

#define RX_FLUSH_TIME_OUT 1500 * 1000 // microseconds timeout
#define KEEPALIVE_IDLE CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT

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

    int opt = 1;
    setsockopt(_listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    esp3d_log("Socket created");

    int err = bind(_listen_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        esp3d_log_e("Socket unable to bind: errno %d", errno);
        esp3d_log_e("IPPROTO: %d", AF_INET);
        close(_listen_socket);
        return false;
    }
    esp3d_log("Socket bound, port %ld", socketServer.port());
    err = listen(_listen_socket, 1);
    if (err != 0) {
        esp3d_log_e("Error occurred during listen: errno %d", errno);
        close(_listen_socket);
        return false;
    }
    return true;
}

// this task only collecting serial RX data and push thenmm to Rx Queue
static void esp3d_socket_rx_task(void *pvParameter)
{
    (void)pvParameter;
    if (socketServer.startSocketServer()) {

        uint64_t startTimeout = 0; // milliseconds
        while (1) {
        }
        /*   (void) pvParameter;
           uint8_t data[ESP3D_SOCKET_RX_BUFFER_SIZE];
           uint8_t buffer[ESP3D_SOCKET_RX_BUFFER_SIZE];
           size_t pos = 0;
           uint64_t startTimeout=0; //microseconds
           while (1) {
              // Delay
               vTaskDelay(pdMS_TO_TICKS(10));
               if (!serialClient.started()) {
                   break;
               }
               int len = uart_read_bytes(ESP3D_SOCKET_PORT, data, (ESP3D_SOCKET_RX_BUFFER_SIZE - 1), 10 / portTICK_PERIOD_MS);
               if (len) {
                   //parse data
                   startTimeout = esp_timer_get_time();
                   for (size_t i = 0; i < len ; i++) {
                       if (pos <ESP3D_SOCKET_RX_BUFFER_SIZE) {
                           buffer[pos]= data[i];
                           pos++;
                       }
                       //if end of char or buffer is full
                       if (serialClient.isEndChar(data[i]) || pos==ESP3D_SOCKET_RX_BUFFER_SIZE) {
                           //create message and push
                           if (!serialClient.pushMsgToRxQueue(buffer, pos)) {
                               //send error
                               esp3d_log_e("Push Message to rx queue failed");
                           }
                           pos=0;
                       }
                   }
               }
               //if no data during a while then send them
               if (esp_timer_get_time()-startTimeout>(RX_FLUSH_TIME_OUT) && pos>0) {
                   if (!serialClient.pushMsgToRxQueue(data, pos)) {
                       //send error
                       esp3d_log_e("Push Message to rx queue failed");
                   }
                   pos=0;
               }
           }*/
    } else {
        esp3d_log_e("Starting socket server failed");
    }
    socketServer.end();
}

bool ESP3DSocketServer::getClient()
{
    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t addr_len = sizeof(source_addr);
    int sock = accept(_listen_socket, (struct sockaddr *)&source_addr, &addr_len);
    if (sock < 0) {
        esp3d_log_e("Unable to accept connection: errno %d", errno);
        return false;
    }
    int client = getFreeClientSlot();
    if (client < 0) {
        esp3d_log_e("Unable to get free client slot");
        return false;
    }
    return true;
}

int ESP3DSocketServer::getFreeClientSlot()
{
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
        if (_clients[s].socketId == -1) {
            return s;
        }
    }
    return -1;
}

bool ESP3DSocketServer::closeSocket(int socketId)
{
    for (uint s = 0; s < ESP3D_MAX_SOCKET_CLIENTS; s++) {
        if (_clients[s].socketId == socketId) {
            shutdown(socketId, 0);
            close(socketId);
            _clients[s].socketId = -1;
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
    _listen_socket = -1;
    memset(_clients, 0, sizeof(esp3d_socket_client_info_t) * ESP3D_MAX_SOCKET_CLIENTS);
}
ESP3DSocketServer::~ESP3DSocketServer()
{
    end();
}

bool ESP3DSocketServer::isEndChar(uint8_t ch)
{
    return ((char)ch == '\n' || (char)ch == '\r');
}

bool ESP3DSocketServer::begin()
{
    end();
    if (esp3d_state_on != (esp3d_state_t)esp3dTFTsettings.readByte(esp3d_socket_on)) {
        esp3d_log("Telnet is not enabled");
        // return true because no error but _started is false
        return true;
    }
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

    _port = esp3dTFTsettings.readUint32(esp3d_socket_port);

    _started = true;
    BaseType_t res = xTaskCreatePinnedToCore(esp3d_socket_rx_task, "esp3d_socket_rx_task", ESP3D_SOCKET_TASK_SIZE, NULL, ESP3D_SOCKET_TASK_PRIORITY, &_xHandle, ESP3D_SOCKET_TASK_CORE);

    if (res == pdPASS && _xHandle) {
        esp3d_log("Created Socket Task");
        esp3d_log("Socket Server started");
        flush();
        return true;
    } else {
        esp3d_log_e("Serial Task creation failed");
        _started = false;
        return false;
    }
}

bool ESP3DSocketServer::pushMsgToRxQueue(const uint8_t *msg, size_t size)
{
    esp3d_msg_t *newMsgPtr = newMsg();
    if (newMsgPtr) {
        if (Esp3DClient::setDataContent(newMsgPtr, msg, size)) {
#if ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
            newMsgPtr->authentication_level = ESP3D_LEVEL_ADMIN;
#endif // ESP3D_DISABLE_SERIAL_AUTHENTICATION
            newMsgPtr->origin = SERIAL_CLIENT;
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
                // TODO:
                // send message
                // size_t len = uart_write_bytes(ESP3D_SOCKET_PORT, msg->data, msg->size);
                // if (len != msg->size) {
                //     esp3d_log_e("Error writing message %s", msg->data);
                // }
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
    _listen_socket = -1;
    memset(_clients, 0, sizeof(esp3d_socket_client_info_t) * ESP3D_MAX_SOCKET_CLIENTS);
}
