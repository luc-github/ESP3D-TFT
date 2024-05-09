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
#include "screens/leveling_screen.h"

#include <lvgl.h>

#include "screens/auto_leveling_screen.h"
#include "components/back_button_component.h"
#include "components/main_container_component.h"
#include "components/spinner_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "screens/manual_leveling_screen.h"
#include "screens/menu_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace levelingScreen {
lv_timer_t *leveling_screen_delay_timer = NULL;
lv_obj_t *btnback = nullptr;
bool auto_leveling = false;

void leveling_screen_auto_on(bool auto_on) { auto_leveling = auto_on; }

ESP3DScreenType leveling_next_screen = ESP3DScreenType::none;

void leveling_screen_delay_timer_cb(lv_timer_t *timer) {
  if (leveling_screen_delay_timer) {
    lv_timer_del(leveling_screen_delay_timer);
    leveling_screen_delay_timer = NULL;
  }

  switch (leveling_next_screen) {
    case ESP3DScreenType::auto_leveling:
      autoLevelingScreen::create();
      break;
    case ESP3DScreenType::manual_leveling:
      manualLevelingScreen::manual_leveling_screen(auto_leveling);
      break;
    case ESP3DScreenType::menu:
      menuScreen::create();
      break;
    default:
      break;
  }
}

void event_button_leveling_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (leveling_screen_delay_timer) return;
  leveling_next_screen = ESP3DScreenType::menu;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (leveling_screen_delay_timer) return;
    leveling_screen_delay_timer = lv_timer_create(
        leveling_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    leveling_screen_delay_timer_cb(NULL);
  }
}

void event_button_manual_handler(lv_event_t *e) {
  esp3d_log("MANUAL Clicked");
  if (leveling_screen_delay_timer) return;
  leveling_next_screen = ESP3DScreenType::manual_leveling;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (leveling_screen_delay_timer) return;
    leveling_screen_delay_timer = lv_timer_create(
        leveling_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    leveling_screen_delay_timer_cb(NULL);
  }
}

void event_button_auto_handler(lv_event_t *e) {
  esp3d_log("AUTO Clicked");
  if (leveling_screen_delay_timer) return;
  leveling_next_screen = ESP3DScreenType::auto_leveling;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (leveling_screen_delay_timer) return;
    leveling_screen_delay_timer = lv_timer_create(
        leveling_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    leveling_screen_delay_timer_cb(NULL);
  }
}

void leveling_screen(bool autoleveling) {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  auto_leveling = autoleveling;
  // Screen creation
  esp3d_log("Leveling screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_del(ui_current_screen);

  btnback = backButton::create(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_leveling_back_handler,
                      LV_EVENT_CLICKED, NULL);

  lv_obj_t *ui_main_container = mainContainer::create(
      ui_new_screen, btnback, ESP3DStyleType::col_container);

  lv_obj_t *ui_buttons_container = lv_obj_create(ui_main_container);
  ESP3DStyle::apply(ui_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_buttons_container);
  lv_obj_clear_flag(ui_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *btn = nullptr;

  // Create button and label for manual leveling
  btn = symbolButton::create(
      ui_buttons_container, LV_SYMBOL_MANUAL "\n" LV_SYMBOL_LEVELING,
      ESP3D_BUTTON_WIDTH, ESP3D_BUTTON_WIDTH);
  lv_obj_add_event_cb(btn, event_button_manual_handler, LV_EVENT_CLICKED, NULL);

  // Create button and label for auto leveling
  btn = symbolButton::create(
      ui_buttons_container, LV_SYMBOL_AUTOMATIC "\n" LV_SYMBOL_LEVELING,
      ESP3D_BUTTON_WIDTH, ESP3D_BUTTON_WIDTH);
  lv_obj_add_event_cb(btn, event_button_auto_handler, LV_EVENT_CLICKED, NULL);
  esp3dTftui.set_current_screen(ESP3DScreenType::leveling);
}
}  // namespace levelingScreen