/*
  esp3d_ws_server

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
#include "esp3d_ws_server.h"
#include "esp3d_settings.h"
#include "esp3d_log.h"
#include "esp3d_hal.h"
#include "tasks_def.h"
#include "esp3d_commands.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

ESP3DWsServer esp3dWsServer;

#define RX_FLUSH_TIME_OUT 1500  // millisecond timeout

#define FREE_SOCKET_HANDLE -1
#define SOCKET_ERROR -1

uint  ESP3DWsServer::clientsConnected()
{
    uint count = 0;
    for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
        if (_clients[s].socketId != FREE_SOCKET_HANDLE) {
            count++;
        }
    }
    return count;
}

esp3d_ws_client_info_t * ESP3DWsServer::getClientInfo(uint index)
{
    if (index<=ESP3D_MAX_WS_CLIENTS) {
        if (_clients[index].socketId !=FREE_SOCKET_HANDLE) {
            return &_clients[index];
        }
    }
    return nullptr;
}

void ESP3DWsServer::process(esp3d_msg_t * msg)
{
    //add to TX queue that will be processed in task
    if (!addTXData(msg)) {
        // delete message as cannot be added to the queue
        Esp3DClient::deleteMsg(msg);
        esp3d_log_e("Failed to add message to tx queue");
    }
}

void ESP3DWsServer::readSockets()
{
    /* int len;
     static uint64_t startTimeout[ESP3D_MAX_WS_CLIENTS];
     static bool initArrays = false;
     if (!_started || !_data || !_buffer) {
         return;
     }
     static size_t pos[ESP3D_MAX_WS_CLIENTS];
     if (!initArrays) {
         initArrays = true;
         for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
             pos[s]=0;
             startTimeout[s]=0;
             if (!_buffer[s]) {
                 esp3d_log_e("Missing buffer for socket %d", s);
                 return;
             }
         }
     }

     for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
         if (_clients[s].socketId != FREE_SOCKET_HANDLE) {
             len = recv(_clients[s].socketId, _data,ESP3D_WS_RX_BUFFER_SIZE, 0);
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
                         if (pos[s] <ESP3D_WS_RX_BUFFER_SIZE) {
                             _buffer[s][pos[s]]= _data[i];
                             (pos[s])++;
                             _buffer[s][pos[s]]=0;
                         }
                         //if end of char or buffer is full
                         if (isEndChar(_data[i]) || pos[s]==ESP3D_WS_RX_BUFFER_SIZE) {
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
     for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++)
         if (esp3d_hal::millis()-startTimeout[s]>(RX_FLUSH_TIME_OUT) && pos[s]>0) {
             if (!pushMsgToRxQueue((const uint8_t*)_buffer[s], pos[s])) {
                 //send error
                 esp3d_log_e("Push Message  %s of size %d to rx queue failed",_buffer[s], pos[s]);
             }
             pos[s]=0;
         }*/

}

// this task only collecting serial RX data and push thenmm to Rx Queue
static void esp3d_ws_rx_task(void *pvParameter)
{
    (void)pvParameter;
    /*if (esp3dWsServer.startSocketServer()) {

        //uint64_t startTimeout = 0; // milliseconds
        while (1) {
            esp3dWsServer.getClient();
            esp3dWsServer.readSockets();
            esp3dWsServer.handle();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    } else {
        esp3d_log_e("Starting socket server failed");
    }*/
    esp3dWsServer.end();
}

bool ESP3DWsServer::sendToSocket(const int sock, const char * data, const size_t len)
{
    /*  int to_write = len;
      //does it need to add time out ?
      while (to_write > 0) {
          int written = send(sock, data + (len - to_write), to_write, 0);
          if (written < 0 && errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK) {
              esp3d_log_e ( "Error occurred during sending %d", errno);
              return false;
          }
          to_write -= written;
      }*/
    return true;
}

bool ESP3DWsServer::getClient()
{
    /* struct sockaddr_storage source_addr;
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
    */

    return true;
}

int ESP3DWsServer::getFreeClientSlot()
{
    for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
        if (_clients[s].socketId == FREE_SOCKET_HANDLE) {
            return s;
        }
    }
    return SOCKET_ERROR;
}

bool ESP3DWsServer::closeSocket(int socketId)
{
    /*if (socketId!=FREE_SOCKET_HANDLE)
        for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
            if (_clients[s].socketId == socketId) {
                shutdown(socketId, 0);
                close(socketId);
                _clients[s].socketId = FREE_SOCKET_HANDLE;
                memset(&_clients[s].source_addr, 0, sizeof(struct sockaddr_storage));
                return true;
            }
        }*/
    return false;
}

ESP3DWsServer::ESP3DWsServer()
{
    _xHandle = NULL;
    _started = false;
    _server = NULL;
    memset(_clients, 0, sizeof(esp3d_ws_client_info_t) * ESP3D_MAX_WS_CLIENTS);
    for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
        _clients[s].socketId = FREE_SOCKET_HANDLE;
    }
    _data = NULL;
    _buffer = NULL;
}
ESP3DWsServer::~ESP3DWsServer()
{
    end();
}

bool ESP3DWsServer::isEndChar(uint8_t ch)
{
    return ((char)ch == '\n' || (char)ch=='\r');
}

esp_err_t ESP3DWsServer::websocket_handler(httpd_req_t *req)
{
    if (!esp3dWsServer.started()) {
        return ESP_FAIL;
    }
    if (req->method == HTTP_GET) {
        esp3d_log("WS Handshake done");
        esp3dWsServer.onOpen(req);
        return ESP_OK;
    }
    return esp3dWsServer.onMessage(req);
}

esp_err_t ESP3DWsServer::onOpen(httpd_req_t *req)
{
    /*_currentFd = httpd_req_to_sockfd(req);
    _req = req;
    std::string tmpstr = "currentID:";
    tmpstr+=std::to_string(_currentFd);
    tmpstr+="\n";
    pushMsgTxt(tmpstr.c_str());
    tmpstr = "activeID:";
    tmpstr+=std::to_string(_currentFd);
    tmpstr+="\n";
    BroadcastTxt(tmpstr.c_str(), _currentFd);*/
    return ESP_OK;
}
esp_err_t ESP3DWsServer::onMessage(httpd_req_t *req)
{
    /* httpd_ws_frame_t ws_pkt;
     uint8_t *buf = NULL;
     memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
     // Set max_len = 0 to get the frame len
     esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
     if (ret != ESP_OK) {
         esp3d_log_e("httpd_ws_recv_frame failed to get frame len with %s",  esp_err_to_name(ret));
         return ret;
     }
     //esp3d_log("frame len is %d", ws_pkt.len);
     if (ws_pkt.len) {
         // ws_pkt.len + 1 is for NULL termination as we are expecting a string
         buf = ( uint8_t *)calloc(1, ws_pkt.len + 1);
         if (buf == NULL) {
             esp3d_log_e("Failed to calloc memory for buf");
             return ESP_ERR_NO_MEM;
         }
         ws_pkt.payload = buf;
         // Set max_len = ws_pkt.len to get the frame payload
         ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
         if (ret != ESP_OK) {
             esp3d_log_e("httpd_ws_recv_frame failed with %s",  esp_err_to_name(ret));
             free(buf);
             return ret;
         }
         if (ws_pkt.type ==HTTPD_WS_TYPE_TEXT) {
             esp3d_log("Got packet with message: %s", ws_pkt.payload);
         } else  if(ws_pkt.type ==HTTPD_WS_TYPE_BINARY) {
             esp3d_log("Got packet with %d bytes", ws_pkt.len);
         } else {
             esp3d_log_e("Unknown frame type %d", ws_pkt.type);
         }
     } else {
         esp3d_log_e("Empty frame type %d", ws_pkt.type);
     }
     free(buf);*/
    return ESP_OK;
}

void ESP3DWsServer::close_fn(httpd_handle_t hd, int socketFd)
{
    esp3d_log("Client closing connection %d", socketFd);
    esp3dWsServer.onClose(socketFd);
    close(socketFd );
    httpd_sess_update_lru_counter(hd,socketFd);
}


esp_err_t ESP3DWsServer::onClose(int fd)
{
    esp3d_log("ID %d is now closed", fd);
    /* if (fd ==_currentFd) {
         _currentFd=-1;
     }*/
    return ESP_OK;
}

bool ESP3DWsServer::begin()
{
    end();
    if (esp3d_state_on != (esp3d_state_t)esp3dTFTsettings.readByte(esp3d_ws_on)) {
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

    _data = (char *) malloc(ESP3D_WS_RX_BUFFER_SIZE+1);
    if (_data == NULL) {
        esp3d_log_e("Memory allocation failed");
        _started = false;
        return false;
    }
    _buffer = (char **) malloc(ESP3D_MAX_WS_CLIENTS*sizeof(char *));
    if (_buffer == NULL) {
        esp3d_log_e("Memory allocation failed");
        _started = false;
        return false;
    }

    for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
        _buffer[s] = (char *) malloc(ESP3D_WS_RX_BUFFER_SIZE+1);
        if (_buffer[s] == NULL) {
            esp3d_log_e("Memory allocation failed");
            _started = false;
            return false;
        }
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    uint32_t  intValue = esp3dTFTsettings.readUint32(esp3d_ws_port);
    //HTTP port
    config.server_port = intValue;
    //Http server core
    config.core_id = 0;
    //stack size (default 4096)
    config.stack_size = 1024*4;
    //Nb of sockets
    config.max_open_sockets = 6; //(3 internals +3)
    //handler
    config.max_uri_handlers = 3; //currently use 10
    //backlog_conn
    config.backlog_conn       = 6;
    //close function
    config.close_fn = close_fn;
    //clear lru
    config.lru_purge_enable = true;

    //start server
    esp3d_log("Starting websocket server on port: '%d'", config.server_port);
    esp_err_t err = httpd_start(&_server, &config);
    if (err == ESP_OK) {
        // Set URI handlers
        esp3d_log("Registering URI handlers");
    }
    //webui web socket /ws
    const httpd_uri_t websocket_handler_config = {
        .uri       = "/ws",
        .method    = HTTP_GET,
        .handler   = (esp_err_t (*)(httpd_req_t*))(esp3dWsServer.websocket_handler),
        .user_ctx  =  nullptr,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = "webui-v3"
    };
    httpd_register_uri_handler(_server, &websocket_handler_config);

    BaseType_t res = xTaskCreatePinnedToCore(esp3d_ws_rx_task, "esp3d_ws_rx_task", ESP3D_WS_TASK_SIZE, NULL, ESP3D_WS_TASK_PRIORITY, &_xHandle, ESP3D_WS_TASK_CORE);

    if (res == pdPASS && _xHandle) {
        esp3d_log("Created WS Task");
        esp3d_log("WebSocket Server started port %ld", _port);
        _started = true;
        return true;
    } else {
        esp3d_log_e("Serial Task creation failed");
        _started = false;
        return false;
    }
}

bool ESP3DWsServer::pushMsgToRxQueue(const uint8_t *msg, size_t size)
{
    esp3d_log("Pushing `%s` %d", msg, size);
    esp3d_msg_t *newMsgPtr = newMsg();
    if (newMsgPtr) {
        if (Esp3DClient::setDataContent(newMsgPtr, msg, size)) {
#if     ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
            newMsgPtr->authentication_level = ESP3D_LEVEL_ADMIN;
#endif // ESP3D_DISABLE_SERIAL_AUTHENTICATION
            newMsgPtr->origin = WEBSOCKET_CLIENT;
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


void ESP3DWsServer::handle()
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
                for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
                    if (_clients[s].socketId != FREE_SOCKET_HANDLE) {
                        sendToSocket(_clients[s].socketId, (const char *)msg->data, msg->size);
                    }
                }
                deleteMsg(msg);
            }
        }
    }
}

void ESP3DWsServer::flush()
{
    uint8_t loopCount = 10;
    while (loopCount && getTxMsgsCount() > 0) {
        // esp3d_log("flushing Tx messages");
        loopCount--;
        handle();
    }
}

void ESP3DWsServer::end()
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
        if (_server) {
            //esp3dWsWebUiService.end();
            httpd_unregister_uri(_server, "/ws");
            httpd_stop(_server);
        }
        _server = nullptr;
        _started = false;
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
        for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
            if(_buffer[s]) {
                free(_buffer[s]);
                _buffer[s] = NULL;
            }
        }
        free(_buffer);
        _buffer = NULL;
    }
    closeAllClients();
}

void ESP3DWsServer::closeAllClients()
{
    for (uint s = 0; s < ESP3D_MAX_WS_CLIENTS; s++) {
        closeSocket(_clients[s].socketId);
    }
}
