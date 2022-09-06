/*
  esp3d-tft

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

#include "esp3d-tft.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "version.h"
#include <string>
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"



Esp3DTFT::Esp3DTFT()
{

}

Esp3DTFT::~Esp3DTFT()
{

}

bool Esp3DTFT::begin()
{
    std::string target =  TFT_TARGET ;
    ESP_LOGI(LOG_TAG, "Starting Esp3DTFT %s ", target.c_str());
    return true;
}

void Esp3DTFT::handle()
{

}

bool Esp3DTFT::end()
{
    return true;
}
