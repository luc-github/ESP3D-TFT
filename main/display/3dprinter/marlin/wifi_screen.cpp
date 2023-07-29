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

#include "wifi_screen.h"

#include <string>

#include "ap_screen.h"
#include "back_button_component.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "main_container_component.h"
#include "menu_screen.h"
#include "sta_screen.h"
#include "symbol_button_component.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace wifiScreen {
lv_timer_t *wifi_screen_delay_timer = NULL;
ESP3DScreenType wifi_next_screen = ESP3DScreenType::none;

void wifi_screen_delay_timer_cb(lv_timer_t *timer) {
  if (wifi_screen_delay_timer) {
    lv_timer_del(wifi_screen_delay_timer);
    wifi_screen_delay_timer = NULL;
  }
  switch (wifi_next_screen) {
    case ESP3DScreenType::access_point:
      apScreen::ap_screen();
      break;
    case ESP3DScreenType::station:
      staScreen::sta_screen();
      break;
    case ESP3DScreenType::menu:
      menuScreen::menu_screen();
      break;
    default:
      break;
  }
}

void event_button_wifi_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::menu;
  if (BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(wifi_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}

void event_button_ap_handler(lv_event_t *e) {
  esp3d_log("AP Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::access_point;
  if (BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(wifi_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}

void event_button_sta_handler(lv_event_t *e) {
  esp3d_log("STA Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::station;
  if (BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(wifi_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}
void event_button_no_wifi_handler(lv_event_t *e) {
  esp3d_log("no wifi Clicked");
}

void wifi_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::wifi);
  // Screen creation
  esp3d_log("Wifi screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_wifi_back_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_t *ui_main_container = mainContainer::create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::col_container);

  lv_obj_t *ui_buttons_container = lv_obj_create(ui_main_container);
  apply_style(ui_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_buttons_container);
  lv_obj_clear_flag(ui_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *btn = nullptr;

  // Create button and label for ap
  btn = symbolButton::create_symbol_button(
      ui_buttons_container, LV_SYMBOL_ACCESS_POINT, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_add_event_cb(btn, event_button_ap_handler, LV_EVENT_CLICKED, NULL);

  // Create button and label for sta
  btn = symbolButton::create_symbol_button(
      ui_buttons_container, LV_SYMBOL_STATION_MODE, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_add_event_cb(btn, event_button_sta_handler, LV_EVENT_CLICKED, NULL);

  // Create button and label for no wifi
  btn = symbolButton::create_symbol_button(ui_buttons_container, LV_SYMBOL_WIFI,
                                           BUTTON_WIDTH, BUTTON_WIDTH, true,
                                           true);
  lv_obj_add_event_cb(btn, event_button_no_wifi_handler, LV_EVENT_CLICKED,
                      NULL);
}
}  // namespace wifiScreen