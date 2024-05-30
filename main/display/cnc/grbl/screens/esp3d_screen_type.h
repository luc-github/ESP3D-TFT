/*
  esp3d_screens_type.h - ESP3D screens type definition

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

enum class ESP3DScreenType : uint8_t {
  none = 0,
  splash,
  main,
  status_list,
  files,
  settings,
  wifi,
  menu,
  informations,
  station,
  access_point,
  positions,
  probe,
  milling,
  laser,
  override,
  macro,
  empty
};
