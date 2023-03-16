/*
  esp3d_network_services

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

#ifdef __cplusplus
extern "C" {
#endif

class ESP3DNetworkServices final {
 public:
  ESP3DNetworkServices();
  ~ESP3DNetworkServices();
  bool begin();
  void handle();
  void end();

  bool started() { return _started; };

 private:
  bool _started;
};
#if ESP3D_WIFI_FEATURE
extern ESP3DNetworkServices esp3dNetworkServices;
#endif  // ESP3D_WIFI_FEATURE
#ifdef __cplusplus
}  // extern "C"
#endif