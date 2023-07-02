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
void menu_screen();
lv_obj_t *create_back_button(lv_obj_t *parent);
lv_obj_t *create_main_container(lv_obj_t *parent, lv_obj_t *button_back,
                                ESP3DStyleType style);
lv_obj_t *create_menu_button(lv_obj_t *container, lv_obj_t *&btn,
                             lv_obj_t *&label, int width = BUTTON_WIDTH,
                             bool center = true);

lv_timer_t *wifi_screen_delay_timer = NULL;

void wifi_screen_delay_timer_cb(lv_timer_t *timer) {
  if (wifi_screen_delay_timer) {
    lv_timer_del(wifi_screen_delay_timer);
    wifi_screen_delay_timer = NULL;
  }
  menu_screen();
}

void event_button_wifi_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  wifi_screen_delay_timer =
      lv_timer_create(wifi_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_AUTO);
  }

  else if (code == LV_EVENT_DEFOCUSED) {
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }
  if (code == LV_EVENT_READY || code == LV_EVENT_DEFOCUSED) {
    esp3d_log("Ready, current text: %s", lv_textarea_get_text(ta));
  }
}

void tp_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_AUTO);
    lv_textarea_set_password_mode(ta, false);
  } else if (code == LV_EVENT_DEFOCUSED) {
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_textarea_set_password_mode(ta, true);
  }
  if (code == LV_EVENT_READY || code == LV_EVENT_DEFOCUSED) {
    esp3d_log("Ready, current text: %s", lv_textarea_get_text(ta));
  }
}

void wifi_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::wifi);
  // Screen creation
  esp3d_log("Wifi screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  // TODO: Add your code here
  lv_obj_t *btnback = create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_wifi_back_handler, LV_EVENT_PRESSED,
                      NULL);
  lv_obj_t *ui_main_container = create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::default_style);

  lv_obj_set_style_bg_opa(ui_main_container, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(ui_main_container, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_border_width(ui_main_container, 0, LV_PART_MAIN);

  lv_obj_t *spinner = lv_spinner_create(ui_new_screen, 1000, 60);
  lv_obj_set_size(spinner, 100, 100);
  lv_obj_center(spinner);
  lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  lv_obj_t *ta;
  lv_obj_t *tp;
  ta = lv_textarea_create(ui_main_container);
  lv_textarea_set_one_line(ta, true);
  lv_textarea_set_max_length(ta, 32);
  tp = lv_textarea_create(ui_main_container);
  lv_textarea_set_password_mode(tp, true);
  lv_textarea_set_password_bullet(tp, "•");
  lv_textarea_set_max_length(tp, 64);
  lv_textarea_set_one_line(tp, true);
  lv_obj_align_to(tp, ta, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);

  lv_obj_t *kb = lv_keyboard_create(ui_main_container);
  lv_keyboard_set_textarea(kb, NULL);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_radius(kb, CURRENT_BUTTON_RADIUS_VALUE, LV_PART_MAIN);
  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
  lv_obj_add_event_cb(tp, tp_event_cb, LV_EVENT_ALL, kb);

  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
}