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

#include "menu_screen.h"

#include <string>

#include "back_button_component.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "filament_screen.h"
#include "informations_screen.h"
#include "leveling_screen.h"
#include "main_container_component.h"
#include "main_screen.h"
#include "menu_button_component.h"
#include "menu_screen.h"
#include "settings_screen.h"
#include "symbol_button_component.h"
#include "wifi_screen.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace menuScreen {
lv_timer_t *menu_screen_delay_timer = NULL;
ESP3DScreenType menu_next_screen = ESP3DScreenType::none;
lv_obj_t *main_btn_leveling = NULL;
lv_obj_t *main_btn_disable_steppers = NULL;

void menu_display_leveling() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::print_status);
  if (label_text == "paused") {
    lv_obj_add_flag(main_btn_leveling, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "printing") {
    lv_obj_add_flag(main_btn_leveling, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(main_btn_leveling, LV_OBJ_FLAG_HIDDEN);
  }
}

void menu_display_disable_steppers() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::print_status);
  if (label_text == "paused") {
    lv_obj_add_flag(main_btn_disable_steppers, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "printing") {
    lv_obj_add_flag(main_btn_disable_steppers, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(main_btn_disable_steppers, LV_OBJ_FLAG_HIDDEN);
  }
}

void menu_screen_print_status_value_cb(ESP3DValuesIndex index,
                                       const char *value,
                                       ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::menu) {
      menu_display_leveling();
      menu_display_disable_steppers();
    }
  }
  // Todo update buttons display according status
}

void menu_screen_delay_timer_cb(lv_timer_t *timer) {
  if (menu_screen_delay_timer) {
    lv_timer_del(menu_screen_delay_timer);
    menu_screen_delay_timer = NULL;
  }
  switch (menu_next_screen) {
    case ESP3DScreenType::main:
      mainScreen::main_screen();
      break;
    case ESP3DScreenType::filament:
      filamentScreen::filament_screen();
      break;
    case ESP3DScreenType::settings:
      settingsScreen::settings_screen();
      break;
    case ESP3DScreenType::leveling:
      levelingScreen::leveling_screen();
      break;
    case ESP3DScreenType::informations:
      informationsScreen::informations_screen();
      break;
    case ESP3DScreenType::wifi:
      wifiScreen::wifi_screen();
      break;
    default:
      break;
  }
}

void event_button_menu_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::main;
  if (BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(menu_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    menu_screen_delay_timer_cb(NULL);
  }
}

void event_button_filament_handler(lv_event_t *e) {
  esp3d_log("filament Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::filament;
  if (BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(menu_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    menu_screen_delay_timer_cb(NULL);
  }
}

void event_button_wifi_handler(lv_event_t *e) {
  esp3d_log("wifi Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::wifi;
  if (BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(menu_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    menu_screen_delay_timer_cb(NULL);
  }
}

void event_button_settings_handler(lv_event_t *e) {
  esp3d_log("settings Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::settings;
  if (BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(menu_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else
    menu_screen_delay_timer_cb(NULL);
}

void event_button_leveling_handler(lv_event_t *e) {
  esp3d_log("leveling Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::leveling;
  if (BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(menu_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else
    menu_screen_delay_timer_cb(NULL);
}

void event_button_informations_handler(lv_event_t *e) {
  esp3d_log("informations Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::informations;
  if (BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(menu_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else
    menu_screen_delay_timer_cb(NULL);
}

void event_button_disable_steppers_handler(lv_event_t *e) {
  esp3d_log("Disable Steppers Clicked");
}

void menu_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("menu screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_menu_back_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_t *ui_main_container = mainContainer::create_main_container(
      ui_new_screen, btnback, ESP3DStyleType::col_container);

  // Add buttons top container to main container
  lv_obj_t *ui_top_buttons_container = lv_obj_create(ui_main_container);
  apply_style(ui_top_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_top_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_top_buttons_container);
  lv_obj_clear_flag(ui_top_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Add buttons bottom container to main container
  lv_obj_t *ui_bottom_buttons_container = lv_obj_create(ui_main_container);
  apply_style(ui_bottom_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_bottom_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_bottom_buttons_container);
  lv_obj_clear_flag(ui_bottom_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  //**********************************

  // Create button and label for filament button
  lv_obj_t *btn1 = symbolButton::create_symbol_button(
      ui_top_buttons_container, LV_SYMBOL_FILAMENT, BUTTON_WIDTH, BUTTON_HEIGHT,
      true, false, 90);
  lv_obj_add_event_cb(btn1, event_button_filament_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for leveling button
  std::string label2 = LV_SYMBOL_LEVELING;
  main_btn_leveling =
      menuButton::create_menu_button(ui_top_buttons_container, label2.c_str());
  lv_obj_add_event_cb(main_btn_leveling, event_button_leveling_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for settings button
  std::string label3 = LV_SYMBOL_SETTINGS;
  lv_obj_t *btn3 =
      menuButton::create_menu_button(ui_top_buttons_container, label3.c_str());
  lv_obj_add_event_cb(btn3, event_button_settings_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for wifi button
  std::string label4 = LV_SYMBOL_WIFI;
  lv_obj_t *btn4 = menuButton::create_menu_button(ui_bottom_buttons_container,
                                                  label4.c_str());
  lv_obj_add_event_cb(btn4, event_button_wifi_handler, LV_EVENT_CLICKED, NULL);

  // Create button and label for disable steppers button
  main_btn_disable_steppers = symbolButton::create_symbol_button(
      ui_bottom_buttons_container, LV_SYMBOL_ENGINE, BUTTON_WIDTH,
      BUTTON_HEIGHT, true, true, 90);
  lv_obj_add_event_cb(main_btn_disable_steppers,
                      event_button_disable_steppers_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for informations button
  std::string label6 = LV_SYMBOL_MORE_INFO;
  lv_obj_t *btn6 = menuButton::create_menu_button(ui_bottom_buttons_container,
                                                  label6.c_str());
  lv_obj_add_event_cb(btn6, event_button_informations_handler, LV_EVENT_CLICKED,
                      NULL);
  esp3dTftui.set_current_screen(ESP3DScreenType::menu);
  menu_display_disable_steppers();
  menu_display_leveling();
}
}  // namespace menuScreen