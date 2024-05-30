/*
  esp3d_tft

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

#include "menu_button_component.h"

#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "symbol_button_component.h"

/**********************
 *  Namespace
 **********************/
namespace menuButton {
lv_obj_t *create(lv_obj_t *container, const char *text) {
  return symbolButton::create(container, text, ESP3D_BUTTON_WIDTH,
                              ESP3D_BUTTON_HEIGHT);
}
}  // namespace menuButton
