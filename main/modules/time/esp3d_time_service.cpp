/*
  esp3d_time_service
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

#include "esp3d_time_service.h"

#include <stdio.h>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_values.h"
#include "esp_netif_types.h"
#include "esp_sntp.h"
#include "network/esp3d_network.h"

TimeService esp3dTimeService;
#define TIMEOUT_NTP_REQUEST 180000  // 3 minutes max to get time
#define TIMEOUT_NTP_REFRESH 5000    // 5 seconds refresh time

TimeService::TimeService() {
  _started = false;
  _is_internet_time = false;
  _time_zone = "+00:00";
  _server_count = 0;
  _dispatch_time = 0;
}
TimeService::~TimeService() { end(); }

bool TimeService::isInternetTime(bool readfromsettings) {
  if (readfromsettings) {
    _is_internet_time =
        esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_use_internet_time);
    esp3d_log("Internet time is %s",
              _is_internet_time ? "enabled" : "disabled");
  }
  return _is_internet_time;
}

const char* TimeService::getTimeZone() { return _time_zone.c_str(); }

bool TimeService::setTimeZone(const char* stime) {
  if (esp3dTftsettings.isValidStringSetting(
          stime, ESP3DSettingIndex::esp3d_timezone)) {
    _time_zone = stime;
    return esp3dTftsettings.writeString(ESP3DSettingIndex::esp3d_timezone,
                                        _time_zone.c_str());
  }
  return false;
}

bool TimeService::begin() {
  bool res = true;
  char out_str[SIZE_OF_SERVER_URL + 1] = {0};
  end();
  // check time zone because it is independent of internet time
  updateTimeZone(true);

  // Check if internet time is enabled
  // if not we do nothing and return true
  if (!isInternetTime(true)) {
    return true;
  }
  // Check if network is available because we need it
  ESP3DRadioMode current_radio_mode = esp3dNetwork.getMode();
  // No network   = no internet time
  if (current_radio_mode == ESP3DRadioMode::off) {
    esp3d_log("No Internet time in OFF mode");
    return false;
  }
  // AP mode does not have internet = no internet time
  if (current_radio_mode == ESP3DRadioMode::wifi_ap ||
      current_radio_mode == ESP3DRadioMode::wifi_ap_config ||
      current_radio_mode == ESP3DRadioMode::wifi_ap_limited) {
    esp3d_log("No Internet time in AP mode");
    return false;
  }
  // BT mode does not have internet = no internet time
  if (current_radio_mode == ESP3DRadioMode::bluetooth_serial) {
    esp3d_log("No Internet time in bt mode");
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
    esp3d_log("No time server defined");
    return false;
  }
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  for (int i = 0; i < _server_count; i++) {
    esp_sntp_setservername(i, _server_url[i].c_str());
    esp3d_log("Time server %d is %s", i, _server_url[i].c_str());
  }

  esp_sntp_init();
  // apply timezone
  updateTimeZone();
  if (_is_internet_time) {
    // Dispatch time async because get server time is not immediate
    _dispatch_time = esp3d_hal::millis();
  } else {
    _dispatch_time = 0;
    esp3d_log("Internet time is disabled");
  }
  if (!res) {
    end();
  }
  _started = res;
  return _started;
}

bool TimeService::updateTimeZone(bool fromsettings) {
  char out_str[SIZE_OF_TIMEZONE + 1] = {0};
  _time_zone = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_timezone,
                                           out_str, SIZE_OF_TIMEZONE);
  if (!esp3dTftsettings.isValidStringSetting(
          _time_zone.c_str(), ESP3DSettingIndex::esp3d_timezone)) {
    esp3d_log_e("Invalid time zone %s", _time_zone.c_str());
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

const char* TimeService::getCurrentTime() {
  struct tm tmstruct;
  time_t now;
  // get current time
  time(&now);
  localtime_r(&now, &tmstruct);
  static char buf[20];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmstruct);
  esp3d_log("Time string is %s", buf);
  return buf;
}

// the string date time  need to be iso-8601
// the time zone part will be ignored
bool TimeService::setTime(const char* stime) {
  esp3d_log("Set time to %s", stime);
  std::string stmp = stime;
  struct tm tmstruct;
  struct timeval time_val = {0, 0};
  memset(&tmstruct, 0, sizeof(struct tm));
  if (strptime(stime, "%Y-%m-%dT%H:%M:%S", &tmstruct) == nullptr) {
    esp3d_log("Invalid time format, try without seconds");
    // allow not to set seconds for lazy guys typing command line
    if (strptime(stime, "%Y-%m-%dT%H:%M", &tmstruct) == nullptr) {
      esp3d_log("Invalid time format");
      return false;
    }
  }
#if ESP3D_TFT_LOG
  char buf[20];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tmstruct);
  esp3d_log("Time string is %s", buf);
#endif  // ESP3D_TFT_LOG

  time_val.tv_usec = 0;
  time_val.tv_sec = mktime(&tmstruct);

  // need to set timezone also
  int offset = _get_time_zone_offset_min();
  struct timezone tz = {offset, 0};
  // now set time to system
  if (settimeofday(&time_val, &tz) == -1) {
    return false;
  }
  return true;
}

bool TimeService::started() { return _started; }

// currently not used
void TimeService::end() {
  _started = false;
  _is_internet_time = false;
  _time_zone = "+00:00";
  for (int i = 0; i < CONFIG_LWIP_SNTP_MAX_SERVERS; i++) {
    _server_url[i] = "";
  }
  _server_count = 0;
  esp_sntp_stop();
}

int TimeService::_get_time_zone_offset_min() {
  int offset = 0;
  int hour = atoi(_time_zone.substr(1, 2).c_str());
  int min = atoi(_time_zone.substr(4, 2).c_str());
  offset = hour * 60 + min;
  // result is in minutes west of GMT
  if (_time_zone[0] == '+' && offset > 0) {
    offset = -offset;
  }
  return offset;
}

// currently not used
void TimeService::handle() {
  static uint32_t last_time = 0;
  if (_started) {
    if (_dispatch_time != 0) {
      if (esp3d_hal::millis() - last_time > TIMEOUT_NTP_REFRESH) {
        last_time = esp3d_hal::millis();
        std::string text = getCurrentTime();
        if (text.find("1970") == std::string::npos) {
          text += "  (" + _time_zone;
          text += ")";
          esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                          text.c_str());
          _dispatch_time = 0;
        } else {
          esp3d_log("Time not set yet");
          if (esp3d_hal::millis() - _dispatch_time > TIMEOUT_NTP_REQUEST) {
            esp3d_log_e("Time failed to set");
            _dispatch_time = 0;
          }
        }
      }
    }
  }
}
