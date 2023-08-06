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

#include "main_screen.h"

#include <string>

#include "empty_screen.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "esp3d_values.h"
#include "fan_screen.h"
#include "files_screen.h"
#include "menu_button_component.h"
#include "menu_screen.h"
#include "message_box_component.h"
#include "positions_screen.h"
#include "speed_screen.h"
#include "status_bar_component.h"
#include "symbol_button_component.h"
#include "temperatures_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace mainScreen {
void main_display_extruder_0();
void main_display_extruder_1();
void main_display_bed();
void main_display_fan();
void main_display_speed();
void main_display_positions();
void main_display_status_area();
void main_display_pause();
void main_display_resume();
void main_display_stop();
void main_display_files();
void main_display_menu();

uint8_t main_screen_temperature_target = 0;
lv_timer_t *main_screen_delay_timer = NULL;
ESP3DScreenType next_screen = ESP3DScreenType::none;

/**********************
 *   STATIC VARIABLES
 **********************/
lv_obj_t *main_btn_extruder_0 = nullptr;
lv_obj_t *main_btn_extruder_1 = nullptr;
lv_obj_t *main_btn_bed = nullptr;
lv_obj_t *main_btn_fan = nullptr;
lv_obj_t *main_btn_speed = nullptr;
lv_obj_t *main_btn_positions = nullptr;
lv_obj_t *main_btn_files = nullptr;
lv_obj_t *main_btn_pause = nullptr;
lv_obj_t *main_btn_stop = nullptr;
lv_obj_t *main_btn_resume = nullptr;
lv_obj_t *main_label_progression_area = nullptr;
lv_obj_t *main_btn_menu = nullptr;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
bool extruder_0_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_extruder_0();
    } else {
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}

bool extruder_1_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_extruder_1();
    } else {
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}

bool bed_value_cb(ESP3DValuesIndex index, const char *value,
                  ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_bed();
    } else {
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}

bool position_value_cb(ESP3DValuesIndex index, const char *value,
                       ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_positions();
    } else {
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}

bool fan_value_cb(ESP3DValuesIndex index, const char *value,
                  ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_fan();
    } else {
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}
bool speed_value_cb(ESP3DValuesIndex index, const char *value,
                    ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_speed();
    } else {
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}
bool print_status_value_cb(ESP3DValuesIndex index, const char *value,
                           ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_status_area();
      main_display_pause();
      main_display_resume();
      main_display_stop();
      main_display_files();
      main_display_menu();
    } else {
      menuScreen::menu_screen_print_status_value_cb(index, value, action);
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}

/**********************
 *   LOCAL FUNCTIONS
 **********************/

void main_display_extruder_0() {
  std::string label_text = esp3dTftValues.get_string_value(
      ESP3DValuesIndex::ext_0_target_temperature);
  std::string tmpstr;
  if (std::stod(label_text) == 0) {
    tmpstr = LV_SYMBOL_EXTRUDER;
  } else {
    tmpstr = LV_SYMBOL_HEAT_EXTRUDER;
  }
  lv_label_set_text_fmt(
      lv_obj_get_child(main_btn_extruder_0, 0), "%s\n%s\n%s1",
      esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_temperature),
      esp3dTftValues.get_string_value(
          ESP3DValuesIndex::ext_0_target_temperature),
      tmpstr.c_str());
}

void main_display_extruder_1() {
  std::string label_text = esp3dTftValues.get_string_value(
      ESP3DValuesIndex::ext_1_target_temperature);
  if (label_text == "#") {
    lv_obj_add_flag(main_btn_extruder_1, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  lv_obj_clear_flag(main_btn_extruder_1, LV_OBJ_FLAG_HIDDEN);
  std::string tmpstr;
  if (std::stod(label_text) == 0) {
    tmpstr = LV_SYMBOL_EXTRUDER;
  } else {
    tmpstr = LV_SYMBOL_HEAT_EXTRUDER;
  }
  lv_label_set_text_fmt(
      lv_obj_get_child(main_btn_extruder_1, 0), "%s\n%s\n%s2",
      esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_1_temperature),
      esp3dTftValues.get_string_value(
          ESP3DValuesIndex::ext_1_target_temperature),
      tmpstr.c_str());
}

void main_display_bed() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::bed_target_temperature);
  if (label_text == "#") {
    lv_obj_add_flag(main_btn_bed, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  lv_obj_clear_flag(main_btn_bed, LV_OBJ_FLAG_HIDDEN);
  std::string tmpstr;
  if (std::stod(label_text) == 0) {
    tmpstr = LV_SYMBOL_NO_HEAT_BED;
  } else {
    tmpstr = LV_SYMBOL_HEAT_BED;
  }
  lv_label_set_text_fmt(
      lv_obj_get_child(main_btn_bed, 0), "%s\n%s\n%s",
      esp3dTftValues.get_string_value(ESP3DValuesIndex::bed_temperature),
      esp3dTftValues.get_string_value(ESP3DValuesIndex::bed_target_temperature),
      tmpstr.c_str());
}

void main_display_positions() {
  lv_label_set_text_fmt(
      lv_obj_get_child(main_btn_positions, 0), "X: %s\nY: %s\nZ: %s",
      esp3dTftValues.get_string_value(ESP3DValuesIndex::x_position),
      esp3dTftValues.get_string_value(ESP3DValuesIndex::y_position),
      esp3dTftValues.get_string_value(ESP3DValuesIndex::z_position));
}

void main_display_status_area() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::print_status);
  if (label_text == "idle") {
    label_text = "Not printing";
  } else {
    if (label_text == "paused")
      label_text = "Printing paused: ";
    else
      label_text = "Printing: ";
    label_text += esp3dTftValues.get_string_value(ESP3DValuesIndex::file_name);
  }
  // To DO : add progress
  lv_label_set_text_fmt(main_label_progression_area, "%s", label_text.c_str());
}

void main_display_fan() {
  // TODO check if need a fan 2
  lv_label_set_text_fmt(
      lv_obj_get_child(main_btn_fan, 0), "%s%%\n%s",
      esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_fan),
      LV_SYMBOL_FAN);
}

void main_display_speed() {
  // TODO check if need a fan 2
  lv_label_set_text_fmt(
      lv_obj_get_child(main_btn_speed, 0), "%s%%\n%s",
      esp3dTftValues.get_string_value(ESP3DValuesIndex::speed),
      LV_SYMBOL_SPEED);
}

void main_display_pause() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::print_status);
  if (label_text == "paused") {
    lv_obj_add_flag(main_btn_pause, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "printing") {
    lv_obj_clear_flag(main_btn_pause, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(main_btn_pause, LV_OBJ_FLAG_HIDDEN);
  }
}

void main_display_resume() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::print_status);
  if (label_text == "paused") {
    lv_obj_clear_flag(main_btn_resume, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "printing") {
    lv_obj_add_flag(main_btn_resume, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(main_btn_resume, LV_OBJ_FLAG_HIDDEN);
  }
}

void main_display_stop() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::print_status);
  if (label_text == "paused") {
    lv_obj_clear_flag(main_btn_stop, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "printing") {
    lv_obj_clear_flag(main_btn_stop, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(main_btn_stop, LV_OBJ_FLAG_HIDDEN);
  }
}

void main_display_files() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::print_status);
  if (label_text == "paused") {
    lv_obj_add_flag(main_btn_files, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "printing") {
    lv_obj_add_flag(main_btn_files, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(main_btn_files, LV_OBJ_FLAG_HIDDEN);
  }
}
void main_display_menu() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::print_status);
  if (label_text == "paused") {
    lv_obj_clear_flag(main_btn_menu, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "printing") {
    lv_obj_add_flag(main_btn_menu, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(main_btn_menu, LV_OBJ_FLAG_HIDDEN);
  }
}

void main_screen_delay_timer_cb(lv_timer_t *timer) {
  // If timer is not null, delete it to avoid multiple call
  if (main_screen_delay_timer) {
    lv_timer_del(main_screen_delay_timer);
    main_screen_delay_timer = NULL;
  }
  switch (next_screen) {
    case ESP3DScreenType::none:
      emptyScreen::empty_screen();
      break;
    case ESP3DScreenType::temperatures:
      temperaturesScreen::temperatures_screen(main_screen_temperature_target);
      break;
    case ESP3DScreenType::positions:
      positionsScreen::positions_screen();
      break;
    case ESP3DScreenType::fan:
      fanScreen::fan_screen();
      break;
    case ESP3DScreenType::speed:
      speedScreen::speed_screen();
      break;
    case ESP3DScreenType::files:
      filesScreen::files_screen();
      break;
    case ESP3DScreenType::menu:
      menuScreen::menu_screen();
      break;

    default:

      break;
  }
  next_screen = ESP3DScreenType::none;
}

void event_button_E0_handler(lv_event_t *e) {
  esp3d_log("E0 Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_screen_temperature_target = 0;
  if (BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(main_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

void event_button_E1_handler(lv_event_t *e) {
  esp3d_log("E1 Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_screen_temperature_target = 1;
  if (BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(main_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

void event_button_Bed_handler(lv_event_t *e) {
  esp3d_log("Bed Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_screen_temperature_target = 2;
  if (BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(main_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

void event_button_positions_handler(lv_event_t *e) {
  esp3d_log("Positions Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::positions;
  if (BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(main_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

void event_button_fan_handler(lv_event_t *e) {
  esp3d_log("Fan Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::fan;
  if (BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(main_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

void event_button_speed_handler(lv_event_t *e) {
  esp3d_log("Speed Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::speed;
  if (BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(main_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

void event_button_files_handler(lv_event_t *e) {
  esp3d_log("Files Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::files;
  if (BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(main_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

void event_button_menu_handler(lv_event_t *e) {
  esp3d_log("Menu Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::menu;
  if (BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(main_screen_delay_timer_cb,
                                              BUTTON_ANIMATION_DELAY, NULL);
  } else
    main_screen_delay_timer_cb(NULL);
}

void event_button_resume_handler(lv_event_t *e) {
  esp3d_log("Resume Clicked");
  esp3dTftValues.set_string_value(ESP3DValuesIndex::print_status, "printing");
}

void event_button_pause_handler(lv_event_t *e) {
  esp3d_log("Pause Clicked");
  esp3dTftValues.set_string_value(ESP3DValuesIndex::print_status, "paused");
}

void event_confirm_stop_cb(lv_event_t *e) {
  lv_obj_t *mbox = lv_event_get_current_target(e);
  std::string rep = lv_msgbox_get_active_btn_text(mbox);
  esp3d_log("Button selectionned : %s", rep == LV_SYMBOL_OK ? "Ok" : "Cancel");
  if (rep == LV_SYMBOL_OK) {
    esp3dTftValues.set_string_value(ESP3DValuesIndex::print_status, "idle");
  }
  lv_msgbox_close(mbox);
}

void event_button_stop_handler(lv_event_t *e) {
  esp3d_log("Stop Clicked");
  std::string text = esp3dTranslationService.translate(ESP3DLabel::stop_print);
  msgBox::confirmationBox(NULL, MsgBoxType::confirmation, text.c_str(),
                          event_confirm_stop_cb);
}

void main_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Main screen creation");
  // Background
  lv_obj_t *ui_main_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_main_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_main_screen, ESP3DStyleType::main_bg);

  lv_obj_t *ui_status_bar_container = statusBar::status_bar(ui_main_screen);
  lv_obj_update_layout(ui_status_bar_container);

  // create container for main screen buttons
  lv_obj_t *ui_container_main_screen = lv_obj_create(ui_main_screen);
  apply_style(ui_container_main_screen, ESP3DStyleType::col_container);
  lv_obj_clear_flag(ui_container_main_screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(ui_container_main_screen, LV_HOR_RES,
                  LV_VER_RES - lv_obj_get_height(ui_status_bar_container));
  lv_obj_align_to(ui_container_main_screen, ui_status_bar_container,
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  // Add buttons top container to main container
  lv_obj_t *ui_top_buttons_container = lv_obj_create(ui_container_main_screen);
  apply_style(ui_top_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_top_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_top_buttons_container);
  lv_obj_clear_flag(ui_top_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Middle container
  lv_obj_t *ui_middle_container = lv_btn_create(ui_container_main_screen);
  apply_style(ui_middle_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_middle_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_middle_container);

  // Add buttons bottom container to main container
  lv_obj_t *ui_bottom_buttons_container =
      lv_obj_create(ui_container_main_screen);
  apply_style(ui_bottom_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_bottom_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_bottom_buttons_container);
  lv_obj_clear_flag(ui_bottom_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  //**********************************
  // Create button and label for Extruder 0
  main_btn_extruder_0 =
      menuButton::create_menu_button(ui_top_buttons_container, "");

  lv_obj_add_event_cb(main_btn_extruder_0, event_button_E0_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for Extruder 1
  main_btn_extruder_1 =
      menuButton::create_menu_button(ui_top_buttons_container, "");

  lv_obj_add_event_cb(main_btn_extruder_1, event_button_E1_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for Bed
  main_btn_bed = menuButton::create_menu_button(ui_top_buttons_container, "");

  lv_obj_add_event_cb(main_btn_bed, event_button_Bed_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for positions
  main_btn_positions = symbolButton::create_symbol_button(
      ui_top_buttons_container, "", BUTTON_WIDTH * 1.5, BUTTON_HEIGHT);

  lv_obj_add_event_cb(main_btn_positions, event_button_positions_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for middle container
  main_label_progression_area = lv_label_create(ui_middle_container);
  apply_style(main_label_progression_area, ESP3DStyleType::status_bar);

  lv_obj_center(main_label_progression_area);
  lv_obj_set_size(main_label_progression_area, CURRENT_STATUS_AREA_WIDTH,
                  CURRENT_STATUS_AREA_HEIGHT);

  // Create button and label for fan
  main_btn_fan =
      menuButton::create_menu_button(ui_bottom_buttons_container, "");

  lv_obj_add_event_cb(main_btn_fan, event_button_fan_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for speed
  main_btn_speed =
      menuButton::create_menu_button(ui_bottom_buttons_container, "");

  lv_obj_add_event_cb(main_btn_speed, event_button_speed_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for pause
  main_btn_pause = menuButton::create_menu_button(ui_bottom_buttons_container,
                                                  LV_SYMBOL_PAUSE);
  lv_obj_add_event_cb(main_btn_pause, event_button_pause_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for resume
  main_btn_resume = menuButton::create_menu_button(ui_bottom_buttons_container,
                                                   LV_SYMBOL_PLAY);
  lv_obj_add_event_cb(main_btn_resume, event_button_resume_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for stop
  main_btn_stop = menuButton::create_menu_button(ui_bottom_buttons_container,
                                                 LV_SYMBOL_STOP);
  lv_obj_add_event_cb(main_btn_stop, event_button_stop_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for files
  main_btn_files = menuButton::create_menu_button(ui_bottom_buttons_container,
                                                  LV_SYMBOL_SD_CARD);
  lv_obj_add_event_cb(main_btn_files, event_button_files_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for menu
  std::string label_text8 = LV_SYMBOL_LIST;
  main_btn_menu = menuButton::create_menu_button(ui_bottom_buttons_container,
                                                 LV_SYMBOL_LIST);
  lv_obj_add_event_cb(main_btn_menu, event_button_menu_handler,
                      LV_EVENT_CLICKED, NULL);
  esp3dTftui.set_current_screen(ESP3DScreenType::main);
  main_display_extruder_0();
  main_display_extruder_1();
  main_display_bed();
  main_display_positions();
  main_display_status_area();
  main_display_pause();
  main_display_resume();
  main_display_files();
  main_display_stop();
  main_display_menu();
  main_display_speed();
  main_display_fan();
}
}  // namespace mainScreen