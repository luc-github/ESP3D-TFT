/*
  esp3d_time_server.h -  time server functions class

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

#pragma once
#include <stdio.h>
#include <time.h>

#include "esp3d_string.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif
class TimeServer final {
 public:
  TimeServer();
  ~TimeServer();
  bool begin();
  void end();
  void handle();
  const char* current_time();
  const char* getTimeZone();
  bool updateTimezone(bool fromsettings = false);
  bool setTime(const char* stime);
  bool started();
  bool isInternetTime(bool readfromsettings = false);

 private:
  bool _started;
  bool _is_internet_time;
  std::string _time_zone;
  std::string _server_url[CONFIG_LWIP_SNTP_MAX_SERVERS];
  uint8_t _server_count;
};

extern TimeServer esp3dTimeService;
#ifdef __cplusplus
}  // extern "C"
#endif