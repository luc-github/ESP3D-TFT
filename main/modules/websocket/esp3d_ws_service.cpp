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
#include "tasks_def.h"
#include <esp_system.h>


#include "freertos/task.h"
#include "esp_timer.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_settings.h"
#include "http/esp3d_http_service.h"

Esp3DWsService esp3dWsWebUiService;


Esp3DWsService::Esp3DWsService()
{
    _started = false;
    _server = nullptr;
    _req = nullptr;
}

Esp3DWsService::~Esp3DWsService() {}

bool Esp3DWsService::begin(httpd_handle_t  serverHandle)
{
    esp3d_log("Starting Ws Service");
    end();
    _server = serverHandle;
    _started= true;
    return _started;
}

void Esp3DWsService::handle() {}

void Esp3DWsService::end()
{
    if (!_started) {
        return;
    }
    esp3d_log("Stop Ws Service");
    closeClients();
    _server = nullptr;
    _started = false;
    _req = nullptr;
}

esp_err_t Esp3DWsService::onOpen(httpd_req_t *req)
{
    int fd = httpd_req_to_sockfd(req);
    _req = req;
    std::string tmpstr = "currentID:";
    tmpstr+=std::to_string(fd);
    tmpstr+="\n";
    pushMsgTxt(tmpstr.c_str());
    tmpstr = "activeID:";
    tmpstr+=std::to_string(fd);
    tmpstr+="\n";
    BroadcastTxt(tmpstr.c_str(), fd);
    return ESP_OK;
}
esp_err_t Esp3DWsService::onMessage(httpd_req_t *req)
{
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        esp3d_log_e("httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    //esp3d_log("frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = ( uint8_t *)calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            esp3d_log_e("Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            esp3d_log_e("httpd_ws_recv_frame failed with %d", ret);
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
    free(buf);
    return ESP_OK;
}
esp_err_t Esp3DWsService::onClose(int fd)
{
    esp3d_log("ID %d is now closed", fd);

    return ESP_OK;
}

void Esp3DWsService::process(esp3d_msg_t * msg)
{
    //TODO : need to handle broadcast error ?
    BroadcastBin(msg->data, msg->size);
    Esp3DClient::deleteMsg(msg);
}

esp_err_t Esp3DWsService::process(httpd_req_t *req)
{
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

esp_err_t Esp3DWsService::pushMsgTxt(const char *msg)
{
    if (!_started || !_req) {
        return ESP_FAIL;
    }
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)msg;
    ws_pkt.len = strlen(msg);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t res = httpd_ws_send_frame(_req, &ws_pkt);
    if (res != ESP_OK) {
        esp3d_log_e("httpd_ws_send_frame failed with %d", res);
    }
    return res;
}

esp_err_t Esp3DWsService::pushMsgBin(uint8_t *msg, size_t len)
{
    if (!_started || !_req) {
        return ESP_FAIL;
    }
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;
    ws_pkt.payload = msg;
    ws_pkt.len = len;
    esp_err_t res = httpd_ws_send_frame(_req, &ws_pkt);
    if (res != ESP_OK) {
        esp3d_log_e("httpd_ws_send_frame failed with %d", res);
    }
    return res;
}

esp_err_t Esp3DWsService::pushMsgTxt(int fd, const char *msg)
{
    return pushMsgTxt(fd, (uint8_t*)msg, strlen(msg));
}

esp_err_t Esp3DWsService::pushMsgTxt(int fd, uint8_t *msg, size_t len)
{
    if (!_started) {
        return ESP_FAIL;
    }
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = msg;
    ws_pkt.len = len;
    esp_err_t res = httpd_ws_send_frame_async(_server, fd, &ws_pkt);
    if (res != ESP_OK) {
        esp3d_log_e("httpd_ws_send_frame failed with %d", res);
    }
    return res;
}
esp_err_t Esp3DWsService::pushMsgBin(int fd, uint8_t *msg, size_t len)
{
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
        esp3d_log_e("httpd_ws_send_frame failed with %d", res);
    }
    return res;
}
esp_err_t Esp3DWsService::BroadcastTxt(const char *msg, int ignore)
{
    return  BroadcastTxt((uint8_t *)msg, strlen(msg), ignore);
}
esp_err_t Esp3DWsService::BroadcastTxt(uint8_t *msg, size_t len, int ignore)
{
    if (!_started) {
        return ESP_FAIL;
    }
    size_t clientCount=MAX_WS_CLIENTS;
    int clientsList[MAX_WS_CLIENTS];
    esp_err_t res = httpd_get_client_list(_server, &clientCount, clientsList);
    if (res == ESP_OK) {
        for (int i = 0; i < clientCount; i++) {
            int fdi = clientsList[i];
            if (httpd_ws_get_fd_info(_server, fdi)==HTTPD_WS_CLIENT_WEBSOCKET && ignore!=fdi) {
                pushMsgTxt(fdi, msg, len);
            }
        }
    }
    return res;
}
esp_err_t Esp3DWsService::BroadcastBin(uint8_t *msg, size_t len, int ignore)
{
    if (!_started) {
        return ESP_FAIL;
    }
    size_t clientCount=MAX_WS_CLIENTS;
    int clientsList[MAX_WS_CLIENTS];
    esp_err_t res = httpd_get_client_list(_server, &clientCount, clientsList);
    if (res == ESP_OK) {
        for (int i = 0; i < clientCount; i++) {
            int fdi = clientsList[i];
            if (httpd_ws_get_fd_info(_server, fdi)==HTTPD_WS_CLIENT_WEBSOCKET  && ignore!=fdi) {
                pushMsgBin(fdi, msg, len);
            }
        }
    }
    return res;
}

void Esp3DWsService::closeClients()
{
    if (!_started) {
        return ;
    }
    size_t clientCount=MAX_WS_CLIENTS;
    int clientsList[MAX_WS_CLIENTS];
    esp_err_t res = httpd_get_client_list(_server, &clientCount, clientsList);
    if (res == ESP_OK) {
        for (int i = 0; i < clientCount; i++) {
            int fdi = clientsList[i];
            if (httpd_ws_get_fd_info(_server, fdi)==HTTPD_WS_CLIENT_WEBSOCKET) {
                httpd_sess_trigger_close(_server, fdi );
            };
        }
    }
}
void Esp3DWsService::enableOnly (int fd)
{
    if (!_started) {
        return ;
    }
    size_t clientCount=MAX_WS_CLIENTS;
    int clientsList[MAX_WS_CLIENTS];
    esp_err_t res = httpd_get_client_list(_server, &clientCount, clientsList);
    if (res == ESP_OK) {
        for (int i = 0; i < clientCount; i++) {
            int fdi = clientsList[i];
            if (httpd_ws_get_fd_info(_server, fdi)==HTTPD_WS_CLIENT_WEBSOCKET && fdi!=fd) {
                httpd_sess_trigger_close(_server, fdi );
            }
        }
    }
}