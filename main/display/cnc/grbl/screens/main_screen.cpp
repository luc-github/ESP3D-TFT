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

#include "screens/main_screen.h"

#include <lvgl.h>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "screens/empty_screen.h"

#if ESP3D_SD_CARD_FEATURE
#include "screens/files_screen.h"
#endif  // ESP3D_SD_CARD_FEATURE
#include "components/menu_button_component.h"
#include "components/message_box_component.h"
#include "components/status_bar_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_json_settings.h"
#include "esp3d_lvgl.h"
#include "gcode_host/esp3d_gcode_host_service.h"
#include "screens/menu_screen.h"

// #include "screens/positions_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  Namespace
 **********************/
namespace mainScreen {
// Static prototypes
// void main_display_positions();
void main_display_status_area();
void main_display_pause();
void main_display_resume();
void main_display_stop();
#if ESP3D_SD_CARD_FEATURE
void main_display_files();
#endif  // ESP3D_SD_CARD_FEATURE
void main_display_menu();

/**********************
 *   STATIC VARIABLES
 **********************/
lv_timer_t *main_screen_delay_timer = NULL;
ESP3DScreenType next_screen = ESP3DScreenType::none;
bool intialization_done = false;
lv_obj_t *main_btn_position_x = nullptr;
lv_obj_t *main_btn_position_y = nullptr;
lv_obj_t *main_btn_position_z = nullptr;
lv_obj_t *main_btn_position_a = nullptr;
lv_obj_t *main_btn_override = nullptr;
lv_obj_t *main_btn_milling = nullptr;

lv_obj_t *main_btn_reset = nullptr;
lv_obj_t *main_btn_switch_positions = nullptr;

lv_obj_t *main_btn_unlock = nullptr;
lv_obj_t *main_btn_abort = nullptr;

lv_obj_t *main_btn_files = nullptr;
lv_obj_t *main_btn_pause = nullptr;
lv_obj_t *main_btn_stop = nullptr;
lv_obj_t *main_btn_resume = nullptr;
lv_obj_t *main_label_progression_area = nullptr;
lv_obj_t *main_btn_menu = nullptr;
std::string progression_area_str = "";
bool display_machine_position = true;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

bool position_values(ESP3DValuesIndex index, const char *value,
                     ESP3DValuesCbAction action) {
  return true;
}
bool state_value_cb(ESP3DValuesIndex index, const char *value,
                    ESP3DValuesCbAction action) {
  return true;
}

bool state_comment_value_cb(ESP3DValuesIndex index, const char *value,
                            ESP3DValuesCbAction action) {
  return true;
}
bool job_status_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    // ESP3DValuesIndex::job_duration, (X)
    // ESP3DValuesIndex::job_progress, (u)
    // ESP3DValuesIndex::job_status (U)
    if (index == ESP3DValuesIndex::job_progress) {
      progression_area_str = "";
      // Display file name
      progression_area_str =
          esp3dTftValues.get_string_value(ESP3DValuesIndex::file_name);
      progression_area_str += "\n";
      progression_area_str += value;
      progression_area_str += "% ";

      // Display time elapsed
      uint64_t time_elapsed = std::stoull(
          esp3dTftValues.get_string_value(ESP3DValuesIndex::job_duration));
      if (time_elapsed > 0) {
        int seconds = time_elapsed / 1000;
        time_elapsed %= 1000;

        int minutes = seconds / 60;
        seconds %= 60;

        int hours = minutes / 60;
        minutes %= 60;

        int days = hours / 24;
        hours %= 24;
        if (days > 0 || hours > 0 || minutes > 0 || seconds > 0) {
          if (days > 0) {
            progression_area_str += std::to_string(days);
            progression_area_str +=
                esp3dTranslationService.translate(ESP3DLabel::days);
            progression_area_str += " ";
          }
          if (hours > 0) {
            if (hours < 10) progression_area_str += "0";
            progression_area_str += std::to_string(hours);
            progression_area_str += ":";
          } else {
            progression_area_str += "00:";
          }

          if (minutes > 0) {
            if (minutes < 10) progression_area_str += "0";
            progression_area_str += std::to_string(minutes);
            progression_area_str += ":";
          } else {
            progression_area_str += "00:";
          }

          if (seconds > 0) {
            if (seconds < 10) progression_area_str += "0";
            progression_area_str += std::to_string(seconds);
          } else {
            progression_area_str += "00";
          }
        } else {
          progression_area_str += std::to_string(time_elapsed);
          esp3dTranslationService.translate(ESP3DLabel::days);
          progression_area_str +=
              esp3dTranslationService.translate(ESP3DLabel::ms);
        }

        esp3d_log("Time elapsed %02dH:%02dMin:%02ds%lld", hours, minutes,
                  seconds, time_elapsed);
        esp3d_log("%s", progression_area_str.c_str());
      }
    }

    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      if (index == ESP3DValuesIndex::job_progress) {
        main_display_status_area();
      }
      if (index == ESP3DValuesIndex::job_status) {
        main_display_pause();
        main_display_resume();
        main_display_stop();
      }
#if ESP3D_SD_CARD_FEATURE
      main_display_files();
#endif  // ESP3D_SD_CARD_FEATURE
      main_display_menu();
    } else {
      menuScreen::job_status_value_cb(index, value, action);
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}

/**********************
 *   LOCAL FUNCTIONS
 **********************/
// void main_display_positions() {
//   lv_label_set_text_fmt(
//       lv_obj_get_child(main_btn_positions, 0), "X: %s\nY: %s\nZ: %s",
//       esp3dTftValues.get_string_value(ESP3DValuesIndex::position_x),
//       esp3dTftValues.get_string_value(ESP3DValuesIndex::position_y),
//       esp3dTftValues.get_string_value(ESP3DValuesIndex::position_z));
// }

void main_display_status_area() {
  if (main_label_progression_area)
    lv_label_set_text(main_label_progression_area,
                      progression_area_str.c_str());
}

void main_display_pause() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::job_status);
  if (label_text == "paused") {
    lv_obj_add_flag(main_btn_pause, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "processing") {
    lv_obj_clear_flag(main_btn_pause, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(main_btn_pause, LV_OBJ_FLAG_HIDDEN);
  }
}

void main_display_resume() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::job_status);
  if (label_text == "paused") {
    lv_obj_clear_flag(main_btn_resume, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "processing") {
    lv_obj_add_flag(main_btn_resume, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(main_btn_resume, LV_OBJ_FLAG_HIDDEN);
  }
}

void main_display_stop() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::job_status);
  if (label_text == "paused") {
    lv_obj_clear_flag(main_btn_stop, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "processing") {
    lv_obj_clear_flag(main_btn_stop, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(main_btn_stop, LV_OBJ_FLAG_HIDDEN);
  }
}

#if ESP3D_SD_CARD_FEATURE

void main_display_files() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::job_status);
  if (label_text == "paused") {
    lv_obj_add_flag(main_btn_files, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "processing") {
    lv_obj_add_flag(main_btn_files, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(main_btn_files, LV_OBJ_FLAG_HIDDEN);
  }
}
#endif  // ESP3D_SD_CARD_FEATURE

void main_display_menu() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::job_status);
  if (label_text == "paused") {
    lv_obj_clear_flag(main_btn_menu, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "processing") {
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
      emptyScreen::create();
      break;
      // case ESP3DScreenType::positions:
      //   positionsScreen::create();
      //   break;
#if ESP3D_SD_CARD_FEATURE
    case ESP3DScreenType::files:
      filesScreen::create();
      break;
#endif  // ESP3D_SD_CARD_FEATURE
    case ESP3DScreenType::menu:
      menuScreen::create();
      break;

    default:

      break;
  }
  next_screen = ESP3DScreenType::none;
}

void event_button_positions_handler(lv_event_t *e) {
  esp3d_log("Positions Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::positions;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(
        main_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

#if ESP3D_SD_CARD_FEATURE
void event_button_files_handler(lv_event_t *e) {
  esp3d_log("Files Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::files;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(
        main_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}
#endif  // ESP3D_SD_CARD_FEATURE

void event_button_menu_handler(lv_event_t *e) {
  esp3d_log("Menu Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::menu;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(
        main_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else
    main_screen_delay_timer_cb(NULL);
}

void event_button_resume_handler(lv_event_t *e) {
  esp3d_log("Resume Clicked");
  gcodeHostService.resume();
}

void event_button_pause_handler(lv_event_t *e) {
  esp3d_log("Pause Clicked");
  gcodeHostService.pause();
}

void event_confirm_stop_cb(lv_event_t *e) {
  lv_obj_t *mbox = lv_event_get_current_target(e);
  std::string rep = lv_msgbox_get_active_btn_text(mbox);
  esp3d_log("Button selectionned : %s", rep == LV_SYMBOL_OK ? "Ok" : "Cancel");
  if (rep == LV_SYMBOL_OK) {
    gcodeHostService.abort();
  }
  lv_msgbox_close(mbox);
}

void event_button_stop_handler(lv_event_t *e) {
  esp3d_log("Stop Clicked");
  std::string text = esp3dTranslationService.translate(ESP3DLabel::stop_print);
  msgBox::confirmation(NULL, MsgBoxType::confirmation, text.c_str(),
                       event_confirm_stop_cb);
}

void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Main screen creation");
  if (!intialization_done) {
    intialization_done = true;
  }
  // Background
  lv_obj_t *ui_main_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_main_screen)) {
    esp3d_log_e("Failed to create main screen");
    return;
  }
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_main_screen);
  ESP3DStyle::apply(ui_main_screen, ESP3DStyleType::main_bg);
  if (lv_obj_is_valid(ui_current_screen)) {
    lv_obj_del(ui_current_screen);
  }
  // Create status bar
  lv_obj_t *ui_status_bar_container = statusBar::create(ui_main_screen);
  if (!lv_obj_is_valid(ui_status_bar_container)) {
    esp3d_log_e("Failed to create status bar");
    return;
  }
  lv_obj_update_layout(ui_status_bar_container);

  // create container for main screen buttons
  lv_obj_t *ui_container_main_screen = lv_obj_create(ui_main_screen);
  if (!lv_obj_is_valid(ui_container_main_screen)) {
    esp3d_log_e("Failed to create main screen container");
    return;
  }
  ESP3DStyle::apply(ui_container_main_screen, ESP3DStyleType::col_container);
  lv_obj_clear_flag(ui_container_main_screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(ui_container_main_screen, LV_HOR_RES,
                  LV_VER_RES - lv_obj_get_height(ui_status_bar_container));
  lv_obj_align_to(ui_container_main_screen, ui_status_bar_container,
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  // Add container to  top container to main container
  lv_obj_t *ui_top_buttons_container = lv_obj_create(ui_container_main_screen);
  if (!lv_obj_is_valid(ui_top_buttons_container)) {
    esp3d_log_e("Failed to create top buttons container");
    return;
  }
  ESP3DStyle::apply(ui_top_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_top_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_top_buttons_container);
  lv_obj_clear_flag(ui_top_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Add container to  top container to main container
  lv_obj_t *ui_positions_buttons_container =
      lv_obj_create(ui_top_buttons_container);
  if (!lv_obj_is_valid(ui_positions_buttons_container)) {
    esp3d_log_e("Failed to create top buttons container");
    return;
  }
  ESP3DStyle::apply(ui_positions_buttons_container,
                    ESP3DStyleType::row_container);
  lv_obj_set_size(ui_positions_buttons_container, ESP3D_PROGRESSION_AREA_WIDTH,
                  LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(ui_positions_buttons_container, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_row(ui_positions_buttons_container, 15, 0);
  // ESP3DStyle::add_pad(ui_positions_buttons_container);
  lv_obj_set_style_bg_color(ui_positions_buttons_container,
                            lv_color_hex(0xFF0000), LV_PART_MAIN);
  // lv_obj_clear_flag(ui_positions_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Create button for positionx
  main_btn_position_x = symbolButton::create(ui_positions_buttons_container,
                                             "X: 0.0000", LV_HOR_RES / 3.5, 60);
  main_btn_position_y = symbolButton::create(ui_positions_buttons_container,
                                             "Y: 0.0000", LV_HOR_RES / 3.5, 60);
  main_btn_position_z = symbolButton::create(ui_positions_buttons_container,
                                             "Z: 0.0000", LV_HOR_RES / 3, 60);
  main_btn_position_a = symbolButton::create(ui_positions_buttons_container,
                                             "A: 0.0000", LV_HOR_RES / 3, 60);
  // lv_obj_add_flag(main_btn_position_y, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(main_btn_position_z, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(main_btn_position_a, LV_OBJ_FLAG_HIDDEN);
  // Create button for switch positions
  main_btn_switch_positions =
      symbolButton::create(ui_top_buttons_container, LV_SYMBOL_M_POSITION);

  // Middle container
  lv_obj_t *ui_middle_container = lv_btn_create(ui_container_main_screen);
  if (!lv_obj_is_valid(ui_middle_container)) {
    esp3d_log_e("Failed to create middle container");
    return;
  }

  ESP3DStyle::apply(ui_middle_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_middle_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_middle_container);

  // Add buttons bottom container to main container
  lv_obj_t *ui_bottom_buttons_container =
      lv_obj_create(ui_container_main_screen);
  if (!lv_obj_is_valid(ui_bottom_buttons_container)) {
    esp3d_log_e("Failed to create bottom buttons container");
    return;
  }
  ESP3DStyle::apply(ui_bottom_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_bottom_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_bottom_buttons_container);
  lv_obj_clear_flag(ui_bottom_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Create buttons for middle container

  // Create button and label for unlock
  main_btn_unlock = symbolButton::create(ui_middle_container, LV_SYMBOL_UNLOCK);
  lv_obj_add_flag(main_btn_unlock, LV_OBJ_FLAG_HIDDEN);

  // Create button and label for abort
  main_btn_abort = symbolButton::create(ui_middle_container, LV_SYMBOL_POWER);
  // lv_obj_add_flag(main_btn_abort, LV_OBJ_FLAG_HIDDEN);

  // Create progression area for middle container
  main_label_progression_area = lv_label_create(ui_middle_container);
  ESP3DStyle::apply(main_label_progression_area,
                    ESP3DStyleType::progression_area);

  // Create button and label for reset
  main_btn_reset = symbolButton::create(ui_middle_container, LV_SYMBOL_RESET);

  main_btn_override =
      symbolButton::create(ui_bottom_buttons_container, LV_SYMBOL_GAUGE);

  main_btn_milling =
      symbolButton::create(ui_bottom_buttons_container, LV_SYMBOL_MILLING);

  // Create button and label for pause
  main_btn_pause =
      symbolButton::create(ui_bottom_buttons_container, LV_SYMBOL_PAUSE);
  lv_obj_add_event_cb(main_btn_pause, event_button_pause_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for resume
  main_btn_resume =
      symbolButton::create(ui_bottom_buttons_container, LV_SYMBOL_PLAY);
  lv_obj_add_event_cb(main_btn_resume, event_button_resume_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for stop
  main_btn_stop =
      symbolButton::create(ui_bottom_buttons_container, LV_SYMBOL_STOP);
  lv_obj_add_event_cb(main_btn_stop, event_button_stop_handler,
                      LV_EVENT_CLICKED, NULL);
#if ESP3D_SD_CARD_FEATURE
  // Create button and label for files
  main_btn_files =
      symbolButton::create(ui_bottom_buttons_container, LV_SYMBOL_SD_CARD);
  lv_obj_add_event_cb(main_btn_files, event_button_files_handler,
                      LV_EVENT_CLICKED, NULL);
#endif  // ESP3D_SD_CARD_FEATURE
  // Create button and label for menu
  std::string label_text8 = LV_SYMBOL_LIST;
  main_btn_menu =
      symbolButton::create(ui_bottom_buttons_container, LV_SYMBOL_LIST);
  lv_obj_add_event_cb(main_btn_menu, event_button_menu_handler,
                      LV_EVENT_CLICKED, NULL);
  esp3dTftui.set_current_screen(ESP3DScreenType::main);
  // main_display_positions();
  main_display_status_area();
  main_display_pause();
  main_display_resume();
#if ESP3D_SD_CARD_FEATURE
  main_display_files();
#endif  // ESP3D_SD_CARD_FEATURE
  main_display_stop();
  main_display_menu();
}
}  // namespace mainScreen