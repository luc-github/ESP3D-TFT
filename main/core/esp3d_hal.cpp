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

#include "esp3d_hal.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int64_t esp3d_hal::micros()
{
    return esp_timer_get_time();
}

int64_t esp3d_hal::millis()
{
    return esp3d_hal::micros() / 1000;
}

int64_t esp3d_hal::seconds()
{
    return esp3d_hal::millis() / 1000;
}

void esp3d_hal::wait(int64_t milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}