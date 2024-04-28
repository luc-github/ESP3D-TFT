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

#include "screens/empty_screen.h"

#include <lvgl.h>

#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "screens/main_screen.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace emptyScreen {
void event_button_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  mainScreen::main_screen();
}

void empty_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Main screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_del(ui_current_screen);

  lv_obj_add_event_cb(ui_new_screen, event_button_handler, LV_EVENT_CLICKED,
                      NULL);
}

}  // namespace emptyScreen