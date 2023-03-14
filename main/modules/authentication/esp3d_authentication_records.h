/*
  esp3d_authentication_records

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

#include <string>

#include "esp3d_authentication_types.h"
#include "esp3d_client_types.h"
#include "lwip/sockets.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ESP3D_AUTHENTICATION_FEATURE
typedef struct {
  Esp3dAuthenticationLevel level;
  int socketId;
  esp3d_clients_t client_type;
  char sessionId[25];
  int64_t last_time;
} esp3d_authentication_record_t;
#endif  // ESP3D_AUTHENTICATION_FEATURE

#ifdef __cplusplus
}  // extern "C"
#endif