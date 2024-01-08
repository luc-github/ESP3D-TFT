/*
    manual_leveling__screen.h - esp3d
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

namespace manualLevelingScreen {
extern void manual_leveling_screen(bool auto_leveling);
extern void update_bed_width(double value);
extern void update_bed_depth(double value);
extern void update_invert_x(bool value);
extern void update_invert_y(bool value);
}  // namespace manualLevelingScreen
