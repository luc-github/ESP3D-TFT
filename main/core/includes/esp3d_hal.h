/*
  esp3d_hal helper functions

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

#ifdef __cplusplus
extern "C" {
#endif

namespace esp3d_hal {
int64_t millis();

int64_t micros();

int64_t seconds();

uint64_t getEfuseMac();

void wait(int64_t milliseconds);

class Esp3dTimout final {
 public:
  Esp3dTimout(int64_t timeout)
      : _timeout(timeout), _start(esp3d_hal::millis()) {}
  bool isTimeout() { return (esp3d_hal::millis() - _start) > _timeout; }
  void reset() { _start = esp3d_hal::millis(); }

 private:
  int64_t _timeout;
  int64_t _start;
};

}  // namespace esp3d_hal
#ifdef __cplusplus
}  // extern "C"
#endif