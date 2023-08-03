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

#include "settings_screen.h"

#include <string>

#include "back_button_component.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "list_line_component.h"
#include "main_container_component.h"
#include "menu_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace settingsScreen {
lv_timer_t *settings_screen_delay_timer = NULL;
lv_obj_t *ui_settings_list_ctl = NULL;

void settings_screen_delay_timer_cb(lv_timer_t *timer) {
  if (settings_screen_delay_timer) {
    lv_timer_del(settings_screen_delay_timer);
    settings_screen_delay_timer = NULL;
  }
  menuScreen::menu_screen();
}

void event_button_settings_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (settings_screen_delay_timer) return;
    settings_screen_delay_timer = lv_timer_create(
        settings_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    settings_screen_delay_timer_cb(NULL);
  }
}

void settings_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Settings screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_settings_back_handler,
                      LV_EVENT_CLICKED, NULL);

  ui_settings_list_ctl = lv_list_create(ui_new_screen);
  lv_obj_clear_flag(ui_settings_list_ctl, LV_OBJ_FLAG_SCROLL_ELASTIC);

  lv_obj_set_size(
      ui_settings_list_ctl, LV_HOR_RES - CURRENT_BUTTON_PRESSED_OUTLINE * 2,
      LV_VER_RES -
          ((3 * CURRENT_BUTTON_PRESSED_OUTLINE) + lv_obj_get_height(btnback)));

  lv_obj_set_pos(ui_settings_list_ctl, CURRENT_BUTTON_PRESSED_OUTLINE,
                 CURRENT_BUTTON_PRESSED_OUTLINE);
  // Hostname
  lv_obj_t *line_container =
      listLine::create_list_line_container(ui_settings_list_ctl);
  std::string LabelStr =
      esp3dTranslationService.translate(ESP3DLabel::hostname);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, false);
  }
  // Extensions
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::extensions);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, false);
  }
  esp3dTftui.set_current_screen(ESP3DScreenType::settings);
}
}  // namespace settingsScreen