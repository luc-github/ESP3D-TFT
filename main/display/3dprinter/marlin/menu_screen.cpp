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
void filament_screen();
void settings_screen();
void leveling_screen();
void informations_screen();
void wifi_screen();

lv_obj_t *create_back_button(lv_obj_t *parent);
lv_obj_t *create_main_container(lv_obj_t *parent, lv_obj_t *button_back);
lv_obj_t *create_menu_button(lv_obj_t *container, lv_obj_t *&btn,
                             lv_obj_t *&label, int width = BUTTON_WIDTH,
                             bool center = true);

lv_timer_t *menu_screen_delay_timer = NULL;
ESP3DScreenType menu_next_screen = ESP3DScreenType::none;

void menu_screen_delay_timer_cb(lv_timer_t *timer) {
  if (menu_screen_delay_timer) {
    lv_timer_del(menu_screen_delay_timer);
    menu_screen_delay_timer = NULL;
  }
  switch (menu_next_screen) {
    case ESP3DScreenType::main:
      main_screen();
      break;
    case ESP3DScreenType::filament:
      filament_screen();
      break;
    case ESP3DScreenType::settings:
      settings_screen();
      break;
    case ESP3DScreenType::leveling:
      leveling_screen();
      break;
    case ESP3DScreenType::informations:
      informations_screen();
      break;
    case ESP3DScreenType::wifi:
      wifi_screen();
      break;
    default:
      break;
  }
}

void event_button_menu_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::main;
  menu_screen_delay_timer =
      lv_timer_create(menu_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_filament_handler(lv_event_t *e) {
  esp3d_log("filament Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::filament;
  menu_screen_delay_timer =
      lv_timer_create(menu_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_wifi_handler(lv_event_t *e) {
  esp3d_log("wifi Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::wifi;
  menu_screen_delay_timer =
      lv_timer_create(menu_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_settings_handler(lv_event_t *e) {
  esp3d_log("settings Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::settings;
  menu_screen_delay_timer =
      lv_timer_create(menu_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_leveling_handler(lv_event_t *e) {
  esp3d_log("leveling Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::leveling;
  menu_screen_delay_timer =
      lv_timer_create(menu_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_informations_handler(lv_event_t *e) {
  esp3d_log("informations Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::informations;
  menu_screen_delay_timer =
      lv_timer_create(menu_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
}

void event_button_disable_stepper_handler(lv_event_t *e) {
  esp3d_log("Disable Steppers Clicked");
}

void menu_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::menu);
  // Screen creation
  esp3d_log("menu screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  // TODO: Add your code here
  lv_obj_t *btnback = create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_menu_back_handler, LV_EVENT_PRESSED,
                      NULL);
  lv_obj_t *ui_main_container = create_main_container(ui_new_screen, btnback);

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
  lv_obj_t *label = nullptr;
  lv_obj_t *btn = nullptr;

  // Create button and label for filament button
  label = create_menu_button(ui_top_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_FILAMENT);
  lv_obj_add_event_cb(btn, event_button_filament_handler, LV_EVENT_PRESSED,
                      NULL);

  // Create button and label for leveling button
  label = create_menu_button(ui_top_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_LEVELING);
  lv_obj_add_event_cb(btn, event_button_filament_handler, LV_EVENT_PRESSED,
                      NULL);

  // Create button and label for settings button
  label = create_menu_button(ui_top_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_SETTINGS);
  lv_obj_add_event_cb(btn, event_button_wifi_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for wifi button
  label = create_menu_button(ui_bottom_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_WIFI);
  lv_obj_add_event_cb(btn, event_button_wifi_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for disable steppers button
  label = create_menu_button(ui_bottom_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_UNLOCK " " LV_SYMBOL_JOG);
  lv_obj_add_event_cb(btn, event_button_wifi_handler, LV_EVENT_PRESSED, NULL);

  // Create button and label for informations button
  label = create_menu_button(ui_bottom_buttons_container, btn, label);
  lv_label_set_text_fmt(label, "%s", LV_SYMBOL_MORE_INFO);
  lv_obj_add_event_cb(btn, event_button_informations_handler, LV_EVENT_PRESSED,
                      NULL);

  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
}
