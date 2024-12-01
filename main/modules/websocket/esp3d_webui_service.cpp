/*
  esp3d_webui_service
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

#include "esp3d_webui_service.h"

#include <stdio.h>

#include "authentication/esp3d_authentication.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "http/esp3d_http_service.h"

ESP3DWebUiService esp3dWsWebUiService;

esp_err_t ESP3DWebUiService::onOpen(httpd_req_t *req) {
  int currentFd = httpd_req_to_sockfd(req);
  std::string tmpstr;
  esp3d_log("New connection %d", currentFd);
  if (clientsConnected() == maxClients()) {
    esp3d_log("Already connected: %d client", clientsConnected());
    ESP3DWebSocketInfos *client = getClientInfos(maxClients() - 1);
    if (client) {
      esp3d_log_w("Already connected: %d client with socket %d",
                  clientsConnected(), client->socket_id);
      int currentSocketId = client->socket_id;
      if (currentSocketId != -1) {
        tmpstr = "activeID:";
        tmpstr += std::to_string(currentFd);
        tmpstr += "\n";
        esp3d_log("%s", tmpstr.c_str());
        while (clientsConnected() == maxClients()) {
          esp3d_log("send %s to socket %d", tmpstr.c_str(), currentSocketId);
          pushMsgTxt(currentSocketId, tmpstr.c_str());
          esp3d_hal::wait(500);
          closeClient(currentSocketId);
          esp3d_hal::wait(10);
        }
      }
    } else {
      esp3d_log_e("No client info");
    }
  } else {
    esp3d_log("No previous client connected");
  }
  // set new connection as current client
  addClient(currentFd);
  // send id to new comer
  tmpstr = "currentID:";
  tmpstr += std::to_string(currentFd);
  tmpstr += "\n";
  esp3d_log("%s", tmpstr.c_str());
  pushMsgTxt(currentFd, tmpstr.c_str());

  return ESP_OK;
}

void ESP3DWebUiService::process(ESP3DMessage *msg) {
  // webui use bin for the stream
  esp3d_log("Processing message");
  BroadcastBin(msg->data, msg->size);
  ESP3DClient::deleteMsg(msg);
}

esp_err_t ESP3DWebUiService::pushNotification(const char *msg) {
  // webui use TXT for internal messages
  std::string tmp = "NOTIFICATION:";
  tmp += msg;
  tmp += "\n";
  return BroadcastTxt(tmp.c_str());
}

esp_err_t ESP3DWebUiService::onMessage(httpd_req_t *req) {
  httpd_ws_frame_t ws_pkt;
  uint8_t *buf = NULL;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
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
#if ESP3D_AUTHENTICATION_FEATURE
      int currentFd = httpd_req_to_sockfd(req);
      esp3d_log("Got packet with message: %s on socket %d", ws_pkt.payload,
                currentFd);
      if (esp3d_string::startsWith((const char *)buf, "PING:")) {
        esp3d_log("Got PING on sessionID %s", (const char *)&buf[5]);
        ESP3DAuthenticationRecord *rec =
            esp3dAuthenthicationService.getRecord((const char *)&buf[5]);
        std::string tmpStr = "PING:";
        uint64_t session = esp3dAuthenthicationService.getSessionTimeout();
        if (rec != NULL) {
          int64_t last = rec->last_time;

          uint64_t now = esp3d_hal::millis();
          int64_t diff = now - last;
          if (diff > session) {
            tmpStr += "0";
          } else {
            tmpStr += std::to_string(session - diff);
          }
        } else {
          tmpStr += "0";
        }
        tmpStr += ":";
        tmpStr += std::to_string(session);
        esp3d_log("Pushing %s", tmpStr.c_str());
        pushMsgTxt(currentFd, tmpStr.c_str());
      }

#endif  // ESP3D_AUTHENTICATION_FEATURE
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
