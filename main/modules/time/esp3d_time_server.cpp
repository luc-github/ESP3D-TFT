/*
  esp3d_time_server
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

#include "esp3d_time_server.h"

#include <stdio.h>

#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp_netif_types.h"
#include "esp_sntp.h"
#include "network/esp3d_network.h"

TimeServer esp3dTimeService;

TimeServer::TimeServer() {
  _started = false;
  _is_internet_time = false;
  _time_zone = "+00:00";
  _server_count = 0;
}
TimeServer::~TimeServer() { end(); }

bool TimeServer::isInternetTime(bool readfromsettings) {
  if (readfromsettings) {
    _is_internet_time =
        esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_use_internet_time);
    esp3d_log_d("Internet time is %s",
                _is_internet_time ? "enabled" : "disabled");
  }
  return _is_internet_time;
}

bool TimeServer::begin() {
  bool res = true;
  char out_str[SIZE_OF_SERVER_URL + 1] = {0};
  end();
  // check time zone because it is independent of internet time
  updateTimezone(true);

  // Check if internet time is enabled
  // if not we do nothing and return true
  if (!isInternetTime(true)) {
    return true;
  }
  // Check if network is available because we need it
  ESP3DRadioMode current_radio_mode = esp3dNetwork.getMode();
  // No network   = no internet time
  if (current_radio_mode == ESP3DRadioMode::off) {
    esp3d_log_d("No Internet time in OFF mode");
    return false;
  }
  // AP mode does not have internet = no internet time
  if (current_radio_mode == ESP3DRadioMode::wifi_ap ||
      current_radio_mode == ESP3DRadioMode::wifi_ap_config ||
      current_radio_mode == ESP3DRadioMode::wifi_ap_limited) {
    esp3d_log_d("No Internet time in AP mode");
    return false;
  }
  // BT mode does not have internet = no internet time
  if (current_radio_mode == ESP3DRadioMode::bluetooth_serial) {
    esp3d_log_d("No Internet time in bt mode");
    return false;
  }

  // TODO: add DHCP time setup option
  //  DHCP = no static IP setup, sta mode and DHCP server providing time

  // now we are sure we are in sta mode
  // get time server address
  // server 1
  std::string stmp = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_time_server1, out_str, SIZE_OF_SERVER_URL);
  if (stmp.length() > 0) {
    _server_url[_server_count] = stmp.c_str();
    _server_count++;
  }
  // server 2
  stmp = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_time_server2,
                                     out_str, SIZE_OF_SERVER_URL);
  if (stmp.length() > 0) {
    _server_url[_server_count] = stmp.c_str();
    _server_count++;
  }
  // server 3
  stmp = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_time_server3,
                                     out_str, SIZE_OF_SERVER_URL);
  if (stmp.length() > 0) {
    _server_url[_server_count] = stmp.c_str();
    _server_count++;
  }

  if (_server_count == 0) {
    esp3d_log_d("No time server defined");
    return false;
  }

  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  for (int i = 0; i < _server_count; i++) {
    esp_sntp_setservername(i, _server_url[i].c_str());
    esp3d_log_d("Time server %d is %s", i, _server_url[i].c_str());
  }

  esp_sntp_init();
  uint8_t retry = 0;
  // Wait for time to be set
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < 20) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp3d_log_d("Wait for synchronization");
    retry++;
  }

  // apply timezone
  updateTimezone();

#if ESP3D_TFT_LOG
  time_t now;
  struct tm timeinfo;
  // get current time
  time(&now);

  // get local time
  localtime_r(&now, &timeinfo);
  esp3d_log_d("Current time is %s", asctime(&timeinfo));
#endif  // ESP3D_TFT_LOG

  /*
  // Set timezone to China Standard Time
      setenv("TZ", "CST-8", 1);
      tzset();
  */
  if (!res) {
    end();
  }
  _started = res;
  return _started;
}

bool TimeServer::updateTimezone(bool fromsettings) {
  char out_str[SIZE_OF_TIMEZONE + 1] = {0};
  _time_zone = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_timezone,
                                           out_str, SIZE_OF_TIMEZONE);
  if (!esp3dTftsettings.isValidStringSetting(
          _time_zone.c_str(), ESP3DSettingIndex::esp3d_timezone)) {
    esp3d_log_d("Invalid time zone %s", _time_zone.c_str());
    _time_zone = "+00:00";
  }
  std::string stmp = _time_zone;
  if (stmp[0] == '+') {
    stmp[0] = '-';
  } else if (stmp[0] == '-') {
    stmp[0] = '+';
  } else {
    return false;
  }
  stmp = "GMT" + stmp;
  setenv("TZ", stmp.c_str(), 1);
  tzset();
  return true;
}

const char* TimeServer::current_time(time_t t) {
  static std::string stmp;
  struct tm tmstruct;
  time_t now;
  stmp = "";
  // get current time
  if (t == 0) {
    esp3d_log_d("No time provided");
    time(&now);
    localtime_r(&now, &tmstruct);
  } else {
    esp3d_log_d("Got time initialization");
    localtime_r(&t, &tmstruct);
  }
  stmp = std::to_string((tmstruct.tm_year) + 1900) + "-";
  if (((tmstruct.tm_mon) + 1) < 10) {
    stmp += "0";
  }
  stmp += std::to_string((tmstruct.tm_mon) + 1) + "-";
  if (tmstruct.tm_mday < 10) {
    stmp += "0";
  }
  stmp += std::to_string(tmstruct.tm_mday) + " ";
  if (tmstruct.tm_hour < 10) {
    stmp += "0";
  }
  stmp += std::to_string(tmstruct.tm_hour) + ":";
  if (tmstruct.tm_min < 10) {
    stmp += "0";
  }
  stmp += std::to_string(tmstruct.tm_min) + ":";
  if (tmstruct.tm_sec < 10) {
    stmp += "0";
  }
  stmp += std::to_string(tmstruct.tm_sec);
  stmp += "  (" + _time_zone;
  stmp += ")";
  esp3d_log_d("Current time is %s", stmp.c_str());
  return stmp.c_str();
}

// the string date time  need to be iso-8601
// the time zone part will be ignored
bool TimeServer::setTime(const char* stime) {
  /*
esp3d_log_d("Set time to %s", stime);
String stmp = stime;
struct tm tmstruct;
struct timeval time_val = {0, 0};
memset(&tmstruct, 0, sizeof(struct tm));
if (strptime(stime, "%Y-%m-%dT%H:%M:%S", &tmstruct) == nullptr) {
  esp3d_log_d("Invalid time format, try without seconds");
  // allow not to set seconds for lazy guys typing command line
  if (strptime(stime, "%Y-%m-%dT%H:%M", &tmstruct) == nullptr) {
    esp3d_log_d("Invalid time format");
    return false;
  }
}
char buf[80];
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmstruct);
esp3d_log_d("Time string is %s", buf);
time_val.tv_usec = 0;
time_val.tv_sec = mktime(&tmstruct);
// try to setTime
if (settimeofday(&time_val, 0) == -1) {
  return false;
}*/
  return true;
}

bool TimeServer::started() { return _started; }

// currently not used
void TimeServer::end() {
  _started = false;
  _is_internet_time = false;
  _time_zone = "+00:00";
  for (int i = 0; i < CONFIG_LWIP_SNTP_MAX_SERVERS; i++) {
    _server_url[i] = "";
  }
  _server_count = 0;
  esp_sntp_stop();
}

// currently not used
void TimeServer::handle() {
  if (_started) {
  }
}
