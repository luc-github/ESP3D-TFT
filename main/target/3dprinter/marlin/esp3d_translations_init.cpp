/*
  esp3d_translation_service
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

#include <cstdlib>
#include <map>
#include <ranges>

#include "translations/esp3d_translation_service.h"

void ESP3DTranslationService::init() {
  _translations = {
      {ESP3DLabel::language, "English"},
      {ESP3DLabel::version, "Version"},
      {ESP3DLabel::size_for_update, "Size for update"},
      {ESP3DLabel::screen, "Screen"},
      {ESP3DLabel::architecture, "Arch"},
      {ESP3DLabel::sdk_version, "SDK"},
      {ESP3DLabel::cpu_freq, "Freq"},
      {ESP3DLabel::flash_size, "Flash"},
      {ESP3DLabel::free_heap, "Free Heap"},
      {ESP3DLabel::total_psram, "PSRAM"},
      {ESP3DLabel::sd_updater, "SD Updater"},
      {ESP3DLabel::on, "On"},
      {ESP3DLabel::off, "Off"},
      {ESP3DLabel::millimeters, "mm"},
      {ESP3DLabel::celcius, "°C"},
      {ESP3DLabel::flash_type, "Flash type"},
      {ESP3DLabel::confirm, "Please confirm"},
      {ESP3DLabel::stop_print, "Do you want to stop current print?"},
      {ESP3DLabel::error, "Error"},
      {ESP3DLabel::error_applying_mode, "Apply mode failed!"},
  };
}
