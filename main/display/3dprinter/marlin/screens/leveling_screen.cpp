/*
leveling_screen.cpp - esp3d

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

#include "leveling_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/main_container_component.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "menu_screen.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace levelingScreen {
lv_timer_t *leveling_screen_delay_timer = NULL;

void leveling_screen_delay_timer_cb(lv_timer_t *timer) {
  if (leveling_screen_delay_timer) {
    lv_timer_del(leveling_screen_delay_timer);
    leveling_screen_delay_timer = NULL;
  }
  menuScreen::menu_screen();
}

void event_button_leveling_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    leveling_screen_delay_timer = lv_timer_create(
        leveling_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    leveling_screen_delay_timer_cb(NULL);
  }
}

void leveling_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Leveling screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  // TODO: Add your code here
  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_leveling_back_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_t *ui_main_container = mainContainer::create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::col_container);

  lv_obj_set_style_bg_opa(ui_main_container, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(ui_main_container, lv_color_white(), LV_PART_MAIN);
  esp3dTftui.set_current_screen(ESP3DScreenType::leveling);
}
}  // namespace levelingScreen