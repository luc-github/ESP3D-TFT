/*
  esp3d_mdns
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
#include "esp3d_mdns.h"

#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_version.h"
#include "network/esp3d_network.h"

#if ESP3D_WS_SERVICE_FEATURE
#include "websocket/esp3d_ws_service.h"
#endif  // ESP3D_WS_SERVICE_FEATURE

#define ESP3D_MDNS_SERVICE_NAME "_esp3d"
#define ESP3D_MDNS_SERVICE_PROTO "_tcp"
#define ESP3D_CODE_BASE "ESP3D-TFT"

ESP3DmDNS esp3dmDNS;

ESP3DmDNS::ESP3DmDNS() {
  _started = false;
  _scanResults = NULL;
  _currentresult = NULL;
  _count = 0;
}

mdns_result_t *ESP3DmDNS::getRecord(int pos) {
  if (!_started || pos < -1 || pos >= _count) {
    return NULL;
  }
  if (pos == -1) {
    mdns_result_t *res = _currentresult;
    if (_currentresult) {
      _currentresult = _currentresult->next;
    }
    return res;
  }
  uint16_t count = 0;
  mdns_result_t *r = _scanResults;
  while (r) {
    if (pos == count) {
      mdns_result_t *res = _currentresult;
      if (_currentresult) {
        _currentresult = _currentresult->next;
      }
      return res;
    }
    count++;
    r = r->next;
  }
  return NULL;
}

uint16_t ESP3DmDNS::servicesScan() {
  freeServiceScan();
  _count = 0;
  esp_err_t err =
      mdns_query_ptr(ESP3D_MDNS_SERVICE_NAME, ESP3D_MDNS_SERVICE_PROTO, 3000,
                     20, &_scanResults);
  if (err) {
    esp3d_log("Query Failed: %s", esp_err_to_name(err));
    return 0;
  }
  if (!_scanResults) {
    esp3d_log("No results found!");
    return 0;
  }
  uint16_t count = 0;
  mdns_result_t *r = _scanResults;
  while (r) {
    count++;
    r = r->next;
  }
  _currentresult = _scanResults;
  _count = count;
  return count;
  return count;
}
void ESP3DmDNS::freeServiceScan() {
  _currentresult = NULL;
  _count = 0;
  if (_scanResults) {
    mdns_query_results_free(_scanResults);
    _scanResults = NULL;
  }
}

ESP3DmDNS::~ESP3DmDNS() { end(); }

bool ESP3DmDNS::begin() {
  if (_started) {
    end();
  }
  esp_err_t err = mdns_init();
  if (err != ESP_OK) {
    esp3d_log_e("Failed to initialize mdns service : %s", esp_err_to_name(err));
    return false;
  }
  err = mdns_hostname_set(esp3dNetwork.getHostName());
  if (err != ESP_OK) {
    mdns_free();
    esp3d_log_e("Failed to set hostname : %s , error:%s",
                esp3dNetwork.getHostName(), esp_err_to_name(err));
    return false;
  }
  err = mdns_instance_name_set(esp3dNetwork.getHostName());
  if (err != ESP_OK) {
    mdns_free();
    esp3d_log_e("Failed to set instance : %s", esp_err_to_name(err));
    return false;
  }
  // structure with TXT records
  mdns_txt_item_t serviceTxtData[2] = {{"firmware", ESP3D_CODE_BASE},
                                       {"version", ESP3D_TFT_VERSION}};
  // to avoid crash if no web service is enabled
  uint32_t httpPort = 80;
#if ESP3D_HTTP_FEATURE
  httpPort = esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_http_port);
#endif  // ESP3D_HTTP_FEATURE
  err = mdns_service_add(esp3dNetwork.getHostName(), ESP3D_MDNS_SERVICE_NAME,
                         ESP3D_MDNS_SERVICE_PROTO, httpPort, serviceTxtData, 2);
#if ESP3D_HTTP_FEATURE
  if (err == ESP_OK) {
    err = mdns_service_add(esp3dNetwork.getHostName(), "_http",
                           ESP3D_MDNS_SERVICE_PROTO, httpPort, NULL, 0);
  }
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_TELNET_FEATURE
  if (err == ESP_OK) {
    uint32_t telnetPort =
        esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_socket_port);
    err = mdns_service_add(esp3dNetwork.getHostName(), "_telnet",
                           ESP3D_MDNS_SERVICE_PROTO, telnetPort, NULL, 0);
  }
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
  if (err == ESP_OK) {
    mdns_txt_item_t serviceTxtData[2] = {
        {"uri", ESP3D_WS_DATA_URL}, {"subprotocol", ESP3D_WS_DATA_SUBPROTOCOL}};
    err =
        mdns_service_add(esp3dNetwork.getHostName(), "_websocket",
                         ESP3D_MDNS_SERVICE_PROTO, httpPort, serviceTxtData, 2);
  }
#endif  // ESP3D_WS_SERVICE_FEATURE

  if (err != ESP_OK) {
    mdns_free();
    esp3d_log_e("Failed to add instance");
    return false;
  }
  _started = true;
  esp3d_log("mDNS Service started");
  return _started;
}

void ESP3DmDNS::handle() {}

void ESP3DmDNS::end() {
  esp3d_log("Stop mDNS service");
  _count = 0;
  freeServiceScan();
  if (_started) {
    mdns_service_remove_all();
    mdns_free();
  }
  _started = false;
}