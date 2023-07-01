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

#include <string>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

void main_screen();
lv_obj_t *create_back_button(lv_obj_t *parent);
lv_obj_t *create_main_container(lv_obj_t *parent, lv_obj_t *button_back);
lv_obj_t *create_menu_button(lv_obj_t *container, lv_obj_t *&btn,
                             lv_obj_t *&label, int width = BUTTON_WIDTH,
                             bool center = true);

lv_timer_t *positions_screen_delay_timer = NULL;

void positions_screen_delay_timer_cb(lv_timer_t *timer) {
  if (positions_screen_delay_timer) {
    lv_timer_del(positions_screen_delay_timer);
    positions_screen_delay_timer = NULL;
  }
  main_screen();
}

void event_button_positions_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  positions_screen_delay_timer = lv_timer_create(
      positions_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void positions_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::positions);
  // Screen creation
  esp3d_log("Temperatures screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  // TODO: Add your code here
  lv_obj_t *btnback = create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_positions_back_handler,
                      LV_EVENT_PRESSED, NULL);
  lv_obj_t *ui_main_container = create_main_container(ui_new_screen, btnback);

  lv_obj_set_style_bg_opa(ui_main_container, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(ui_main_container, lv_color_white(), LV_PART_MAIN);

  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
}
