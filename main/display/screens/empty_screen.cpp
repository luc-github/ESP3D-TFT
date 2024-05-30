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
 *  Namespace
 **********************/
namespace emptyScreen {

/**
 * Handles the event when a button is clicked.
 *
 * @param e The event object containing information about the event.
 */
void event_button_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  mainScreen::create();
}

/**
 * @brief Creates a new screen and sets it as the current screen.
 *
 * This function creates a new screen, sets it as the current screen, and
 * applies the main background style to it. The previous screen is deleted.
 *
 * @note This function assumes that the `esp3dTftui` object and the `esp3d_log`
 * function are defined and accessible.
 *
 * @param None
 * @return None
 */
void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Main screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_new_screen)) {
    esp3d_log_e("Failed to create new screen");
    return;
  }
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);

  if (lv_obj_is_valid(ui_current_screen)) {
    lv_obj_del(ui_current_screen);
  }

  lv_obj_add_event_cb(ui_new_screen, event_button_handler, LV_EVENT_CLICKED,
                      NULL);
}

}  // namespace emptyScreen