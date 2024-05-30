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

#include "screens/menu_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/main_container_component.h"
#include "components/menu_button_component.h"
#include "components/message_box_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "esp3d_values.h"
#include "rendering/esp3d_rendering_client.h"
#include "screens/informations_screen.h"
#include "screens/main_screen.h"
#include "screens/menu_screen.h"
#include "screens/settings_screen.h"
#include "translations/esp3d_translation_service.h"

#if ESP3D_WIFI_FEATURE
#include "screens/wifi_screen.h"
#endif  // ESP3D_WIFI_FEATURE

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace menuScreen {
lv_timer_t *menu_screen_delay_timer = NULL;
ESP3DScreenType menu_next_screen = ESP3DScreenType::none;
bool intialization_done = false;

void job_status_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::menu) {
      // Update buttons display according status
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
      mainScreen::create();
      break;

    case ESP3DScreenType::settings:
      settingsScreen::create();
      break;

    case ESP3DScreenType::informations:
      informationsScreen::create();
      break;
#if ESP3D_WIFI_FEATURE
    case ESP3DScreenType::wifi:
      wifiScreen::create();
      break;
#endif  // ESP3D_WIFI_FEATURE
    default:
      break;
  }
}

void event_button_menu_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::main;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(
        menu_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    menu_screen_delay_timer_cb(NULL);
  }
}

#if ESP3D_WIFI_FEATURE
void event_button_wifi_handler(lv_event_t *e) {
  esp3d_log("wifi Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::wifi;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(
        menu_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    menu_screen_delay_timer_cb(NULL);
  }
}
#endif  // ESP3D_WIFI_FEATURE

void event_button_settings_handler(lv_event_t *e) {
  esp3d_log("settings Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::settings;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(
        menu_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else
    menu_screen_delay_timer_cb(NULL);
}

void event_button_informations_handler(lv_event_t *e) {
  esp3d_log("informations Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::informations;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(
        menu_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else
    menu_screen_delay_timer_cb(NULL);
}

void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  if (!intialization_done) {
    esp3d_log("menu screen initialization");

    intialization_done = true;
  }
  // Screen creation
  esp3d_log("menu screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_del(ui_current_screen);

  lv_obj_t *btnback = backButton::create(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_menu_back_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_t *ui_main_container = mainContainer::create(
      ui_new_screen, btnback, ESP3DStyleType::col_container);

  // Add buttons top container to main container
  lv_obj_t *ui_top_buttons_container = lv_obj_create(ui_main_container);
  ESP3DStyle::apply(ui_top_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_top_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_top_buttons_container);
  lv_obj_clear_flag(ui_top_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Add buttons bottom container to main container
  lv_obj_t *ui_bottom_buttons_container = lv_obj_create(ui_main_container);
  ESP3DStyle::apply(ui_bottom_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_bottom_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_bottom_buttons_container);
  lv_obj_clear_flag(ui_bottom_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  //**********************************

  // Create button and label for filament button
  // lv_obj_t *btn1 = symbolButton::create(
  //     ui_top_buttons_container, LV_SYMBOL_FILAMENT, ESP3D_BUTTON_WIDTH,
  //     ESP3D_BUTTON_HEIGHT, true, false, 90);
  // lv_obj_add_event_cb(btn1, event_button_filament_handler, LV_EVENT_CLICKED,
  //                     NULL);

#if ESP3D_WIFI_FEATURE
  // Create button and label for wifi button
  std::string label4 = LV_SYMBOL_WIFI;
  lv_obj_t *btn4 =
      menuButton::create(ui_bottom_buttons_container, label4.c_str());
  lv_obj_add_event_cb(btn4, event_button_wifi_handler, LV_EVENT_CLICKED, NULL);
#endif  // ESP3D_WIFI_FEATURE

  // Create button and label for settings button
  std::string label3 = LV_SYMBOL_SETTINGS;
  lv_obj_t *btn3 =
      menuButton::create(ui_bottom_buttons_container, label3.c_str());
  lv_obj_add_event_cb(btn3, event_button_settings_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for informations button
  std::string label6 = LV_SYMBOL_MORE_INFO;
  lv_obj_t *btn6 =
      menuButton::create(ui_bottom_buttons_container, label6.c_str());
  lv_obj_add_event_cb(btn6, event_button_informations_handler, LV_EVENT_CLICKED,
                      NULL);
  esp3dTftui.set_current_screen(ESP3DScreenType::menu);
}
}  // namespace menuScreen