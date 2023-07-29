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

#include "sta_screen.h"

#include <string>

#include "back_button_component.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "main_container_component.h"
#include "symbol_button_component.h"
#include "wifi_screen.h"


/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace staScreen {
lv_timer_t *sta_screen_delay_timer = NULL;
lv_obj_t *sta_ta_ssid = NULL;
lv_obj_t *sta_ta_password = NULL;
lv_obj_t *ui_sta_ssid_list_ctl = NULL;
lv_obj_t *sta_spinner = NULL;
lv_obj_t *sta_connection_status = NULL;
lv_obj_t *sta_connection_type = NULL;

void sta_screen_delay_timer_cb(lv_timer_t *timer) {
  if (sta_screen_delay_timer) {
    lv_timer_del(sta_screen_delay_timer);
    sta_screen_delay_timer = NULL;
  }
  wifiScreen::wifi_screen();
}

void event_button_sta_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  sta_screen_delay_timer =
      lv_timer_create(sta_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void sta_ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_AUTO);
    if (ta == sta_ta_password) {
      lv_textarea_set_password_mode(ta, false);
    }
    if (ui_sta_ssid_list_ctl)
      lv_obj_add_flag(ui_sta_ssid_list_ctl, LV_OBJ_FLAG_HIDDEN);
    if (sta_spinner) lv_obj_add_flag(sta_spinner, LV_OBJ_FLAG_HIDDEN);

  }

  else if (code == LV_EVENT_DEFOCUSED) {
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    if (ta == sta_ta_password) {
      lv_textarea_set_password_mode(ta, true);
    }
  } else if (code == LV_EVENT_READY || code == LV_EVENT_DEFOCUSED) {
    if (ta == sta_ta_ssid) {
      esp3d_log("Ready, SSID: %s", lv_textarea_get_text(ta));
    } else if (ta == sta_ta_password) {
      esp3d_log("Ready, PASSWORD: %s", lv_textarea_get_text(ta));
    }
  }
}

void sta_event_button_search_handler(lv_event_t *e) {
  esp3d_log("search Clicked");
  if (ui_sta_ssid_list_ctl) {
    lv_obj_clear_flag(ui_sta_ssid_list_ctl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sta_spinner, LV_OBJ_FLAG_HIDDEN);
  }
}

void sta_event_button_ok_handler(lv_event_t *e) { esp3d_log("Ok Clicked"); }

void sta_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("STA screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_sta_back_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_t *ui_main_container = mainContainer::create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::simple_container);

  // SSID
  lv_obj_t *label_ssid = lv_label_create(ui_main_container);
  lv_label_set_text(label_ssid, LV_SYMBOL_ACCESS_POINT);
  apply_style(label_ssid, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label_ssid);
  int width_label = lv_obj_get_width(label_ssid);
  esp3d_log("width_label %d", lv_obj_get_width(label_ssid));
  sta_ta_ssid = lv_textarea_create(ui_main_container);
  lv_textarea_set_one_line(sta_ta_ssid, true);
  lv_textarea_set_max_length(sta_ta_ssid, 32);
  lv_obj_align(sta_ta_ssid, LV_ALIGN_TOP_LEFT,
               width_label + (CURRENT_BUTTON_PRESSED_OUTLINE * 2), 0);
  lv_obj_set_width(sta_ta_ssid, (LV_HOR_RES / 2));
  lv_obj_align_to(label_ssid, sta_ta_ssid, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);

  // Password
  lv_obj_t *label_pwd = lv_label_create(ui_main_container);
  lv_label_set_text(label_pwd, LV_SYMBOL_UNLOCK);
  apply_style(label_pwd, ESP3DStyleType::bg_label);

  sta_ta_password = lv_textarea_create(ui_main_container);
  lv_textarea_set_password_mode(sta_ta_password, true);
  lv_textarea_set_password_bullet(sta_ta_password, "•");
  lv_textarea_set_max_length(sta_ta_password, 64);
  lv_textarea_set_one_line(sta_ta_password, true);
  lv_obj_set_width(sta_ta_password, (LV_HOR_RES / 2));

  lv_obj_align_to(sta_ta_password, sta_ta_ssid, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_align_to(
      label_pwd, sta_ta_password, LV_ALIGN_OUT_LEFT_MID,
      -(CURRENT_BUTTON_PRESSED_OUTLINE + lv_obj_get_width(label_pwd) / 2), 0);
  // Keyboard
  lv_obj_t *kb = lv_keyboard_create(ui_main_container);
  lv_keyboard_set_textarea(kb, NULL);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_radius(kb, CURRENT_BUTTON_RADIUS_VALUE, LV_PART_MAIN);
  lv_obj_add_event_cb(sta_ta_ssid, sta_ta_event_cb, LV_EVENT_ALL, kb);
  lv_obj_add_event_cb(sta_ta_password, sta_ta_event_cb, LV_EVENT_ALL, kb);

  // SSID list
  ui_sta_ssid_list_ctl = lv_list_create(ui_new_screen);
  lv_obj_update_layout(sta_ta_password);
  lv_obj_align_to(
      ui_sta_ssid_list_ctl, sta_ta_password, LV_ALIGN_OUT_BOTTOM_LEFT,
      -(2 * CURRENT_BUTTON_PRESSED_OUTLINE), (CURRENT_BUTTON_PRESSED_OUTLINE));

  lv_obj_set_width(ui_sta_ssid_list_ctl,
                   (LV_HOR_RES / 2) + (3 * CURRENT_BUTTON_PRESSED_OUTLINE));
  lv_obj_update_layout(ui_sta_ssid_list_ctl);

  lv_obj_set_height(
      ui_sta_ssid_list_ctl,
      lv_obj_get_y(btnback) -
          (lv_obj_get_y(sta_ta_password) + lv_obj_get_height(sta_ta_password)));
  lv_obj_update_layout(ui_sta_ssid_list_ctl);

  // sta_spinner
  sta_spinner = lv_spinner_create(ui_new_screen, 1000, 60);
  lv_obj_set_size(sta_spinner, CURRENT_SPINNER_SIZE, CURRENT_SPINNER_SIZE);
  lv_obj_set_x(sta_spinner, lv_obj_get_x(ui_sta_ssid_list_ctl) +
                                (lv_obj_get_width(ui_sta_ssid_list_ctl) / 2) -
                                (CURRENT_SPINNER_SIZE / 2));
  lv_obj_set_y(sta_spinner, lv_obj_get_y(ui_sta_ssid_list_ctl) +
                                (lv_obj_get_height(ui_sta_ssid_list_ctl) / 2) -
                                (CURRENT_SPINNER_SIZE / 2));
  lv_obj_add_flag(sta_spinner, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_sta_ssid_list_ctl, LV_OBJ_FLAG_HIDDEN);

  // Connection status
  sta_connection_status = lv_led_create(ui_new_screen);
  lv_led_set_brightness(sta_connection_status, 255);
  lv_obj_align_to(sta_connection_status, btnback, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_led_set_color(sta_connection_status, lv_palette_main(LV_PALETTE_RED));

  // Connection type
  sta_connection_type = lv_label_create(ui_new_screen);
  lv_obj_align_to(sta_connection_type, sta_connection_status,
                  LV_ALIGN_OUT_LEFT_MID, 0, 0);
  lv_label_set_text(sta_connection_type, LV_SYMBOL_STATION_MODE);

  lv_obj_t *btn = nullptr;

  // Create button and label for search
  btn = symbolButton::create_symbol_button(ui_main_container, LV_SYMBOL_OK,
                                           SYMBOL_BUTTON_WIDTH,
                                           SYMBOL_BUTTON_WIDTH);

  lv_obj_add_event_cb(btn, sta_event_button_ok_handler, LV_EVENT_CLICKED, NULL);
  lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 0);

  // Create button and label for apply
  lv_obj_t *btn2 = nullptr;
  btn2 = symbolButton::create_symbol_button(ui_main_container, LV_SYMBOL_SEARCH,
                                            SYMBOL_BUTTON_WIDTH,
                                            SYMBOL_BUTTON_WIDTH);

  lv_obj_add_event_cb(btn2, sta_event_button_search_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_align_to(btn2, btn, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  esp3dTftui.set_current_screen(ESP3DScreenType::wifi);
}
}  // namespace staScreen