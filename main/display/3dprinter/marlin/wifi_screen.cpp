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
#include "esp3d_settings.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
void menu_screen();
void ap_screen();
void sta_screen();
lv_obj_t *create_back_button(lv_obj_t *parent);
lv_obj_t *create_main_container(lv_obj_t *parent, lv_obj_t *button_back,
                                ESP3DStyleType style);
lv_obj_t *create_symbol_button(lv_obj_t *container, lv_obj_t *&btn,
                               lv_obj_t *&label,
                               int width = SYMBOL_BUTTON_WIDTH,
                               int height = SYMBOL_BUTTON_HEIGHT,
                               bool center = true, bool slash = false,
                               int rotation = 0);
lv_timer_t *wifi_screen_delay_timer = NULL;
ESP3DScreenType wifi_next_screen = ESP3DScreenType::none;

void wifi_screen_delay_timer_cb(lv_timer_t *timer) {
  if (wifi_screen_delay_timer) {
    lv_timer_del(wifi_screen_delay_timer);
    wifi_screen_delay_timer = NULL;
  }
  switch (wifi_next_screen) {
    case ESP3DScreenType::access_point:
      ap_screen();
      break;
    case ESP3DScreenType::station:
      sta_screen();
      break;
    case ESP3DScreenType::menu:
      menu_screen();
      break;
    default:
      break;
  }
}

void event_button_wifi_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::menu;
  wifi_screen_delay_timer =
      lv_timer_create(wifi_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_ap_handler(lv_event_t *e) {
  esp3d_log("AP Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::access_point;
  wifi_screen_delay_timer =
      lv_timer_create(wifi_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_sta_handler(lv_event_t *e) {
  esp3d_log("STA Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::station;
  wifi_screen_delay_timer =
      lv_timer_create(wifi_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}
void event_button_no_wifi_handler(lv_event_t *e) {
  esp3d_log("no wifi Clicked");
}

void wifi_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::wifi);
  // Screen creation
  esp3d_log("Wifi screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  lv_obj_t *btnback = create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_wifi_back_handler, LV_EVENT_PRESSED,
                      NULL);
  lv_obj_t *ui_main_container = create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::col_container);

  lv_obj_t *ui_buttons_container = lv_obj_create(ui_main_container);
  apply_style(ui_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_buttons_container);
  lv_obj_clear_flag(ui_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *label = nullptr;
  lv_obj_t *btn = nullptr;

  // Create button and label for ap
  label = create_symbol_button(ui_buttons_container, btn, label, BUTTON_WIDTH,
                               BUTTON_WIDTH);
  lv_label_set_text(label, LV_SYMBOL_ACCESS_POINT);
  lv_obj_add_event_cb(btn, event_button_ap_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for sta
  label = create_symbol_button(ui_buttons_container, btn, label, BUTTON_WIDTH,
                               BUTTON_WIDTH);
  lv_label_set_text(label, LV_SYMBOL_STATION_MODE);
  lv_obj_add_event_cb(btn, event_button_sta_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for no wifi
  label = create_symbol_button(ui_buttons_container, btn, label, BUTTON_WIDTH,
                               BUTTON_WIDTH, true, true);
  lv_label_set_text(label, LV_SYMBOL_WIFI);
  lv_obj_add_event_cb(btn, event_button_no_wifi_handler, LV_EVENT_PRESSED,
                      NULL);

  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
}