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
#include "screens/fan_screen.h"

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
#include "screens/filament_screen.h"
#include "screens/menu_screen.h"
#include "screens/positions_screen.h"
#include "screens/speed_screen.h"
#include "screens/temperatures_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 * Namespace
 **********************/
namespace mainScreen {
/**********************
 *   STATIC FUNCTIONS PROTOTYPES
 **********************/
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
#if ESP3D_SD_CARD_FEATURE
void main_display_files();
#endif  // ESP3D_SD_CARD_FEATURE
void main_display_menu();

/**********************
 *   STATIC VARIABLES
 **********************/
uint8_t main_screen_temperature_target = 0;
lv_timer_t *main_screen_delay_timer = NULL;
ESP3DScreenType next_screen = ESP3DScreenType::none;
uint8_t nb_fans = 2;
bool show_fan_button = true;
bool intialization_done = false;

std::string progression_area_str = "";
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

/**
 * @brief Sets the visibility of the fan controls on the main screen.
 *
 * This function is used to show or hide the fan controls on the main screen.
 *
 * @param show Boolean value indicating whether to show or hide the fan
 * controls.
 */
void show_fan_controls(bool show) { show_fan_button = show; }

/**
 * Callback function for handling updates to the value of extruder 0.
 *
 * @param index The index of the value being updated.
 * @param value The new value of the extruder 0.
 * @param action The action to be performed on the value.
 * @return True if the callback was successfully handled, false otherwise.
 */
bool extruder_0_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_extruder_0();
    } else {
      // update other screens calling each callback update function
      temperaturesScreen::extruder_0_value_cb(index, value, action);
      filamentScreen::callback(index, value, action);
    }
  }
  return true;
}

/**
 * Callback function for handling updates to the value of extruder 1.
 *
 * @param index The index of the value being updated.
 * @param value The new value of the extruder 1.
 * @param action The action to be performed on the value.
 * @return True if the callback was successful, false otherwise.
 */
bool extruder_1_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_extruder_1();
      callback(index, value, action);
    } else {
      //  update other screens calling each callback update function
      temperaturesScreen::extruder_1_value_cb(index, value, action);
      fanScreen::callback(index, value, action);
      filamentScreen::callback(index, value, action);
    }
  }
  return true;
}

/**
 * Callback function for updating the bed value in the main screen.
 *
 * @param index The index of the value being updated.
 * @param value The new value for the bed.
 * @param action The action being performed on the value.
 * @return True if the callback was successful, false otherwise.
 */
bool bed_value_cb(ESP3DValuesIndex index, const char *value,
                  ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_bed();
    } else {
      // update other screens calling each callback update function
      temperaturesScreen::bed_value_cb(index, value, action);
    }
  }
  return true;
}

/**
 * Callback function for handling position values in the main screen.
 *
 * @param index The index of the position value.
 * @param value The value of the position.
 * @param action The action to be performed on the position value.
 * @return True if the callback function is executed successfully, false
 * otherwise.
 */
bool position_value_cb(ESP3DValuesIndex index, const char *value,
                       ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_positions();
    } else {
      positionsScreen::callback(index, value, action);
    }
  }
  return true;
}

/**
 * Callback function for handling fan values.
 *
 * This function is called when there is a change in fan values. It checks if
 * the fan control button should be shown, and if so, updates the number of fans
 * based on the extruder data. If the current screen is the main screen, it
 * calls the `main_display_fan` function to update the fan display. Otherwise,
 * it delegates the callback to the `fanScreen` class.
 *
 * @param index The index of the ESP3D value.
 * @param value The value of the ESP3D value.
 * @param action The action to be performed on the ESP3D value.
 * @return True if the callback was handled successfully, false otherwise.
 */
bool callback(ESP3DValuesIndex index, const char *value,
              ESP3DValuesCbAction action) {
  esp3d_log("callback");
  if (!show_fan_button) {
    esp3d_log("No control to show");
    return false;
  }

  if (action == ESP3DValuesCbAction::Update) {
    esp3d_log("Check if main screen");
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      // this call back is called for each fan value, so check if need to update
      // and for each extruder 1 update, so check if need to update nb of fans
      if (index == ESP3DValuesIndex::ext_1_temperature ||
          index == ESP3DValuesIndex::ext_1_target_temperature) {
        esp3d_log("Check if extruder 1 data %s:", value);
        uint nb_fans_tmp = 2;
        if (strcmp(value, "#") == 0) {
          esp3d_log("No extruder 1, only one fan");
          nb_fans_tmp = 1;
        }
        if (nb_fans_tmp != nb_fans) {
          esp3d_log("Update nb of fans");
          nb_fans = nb_fans_tmp;
        } else {
          // no update needed
          esp3d_log("No update needed");
          return false;
        }
      }
      main_display_fan();
    } else {
      fanScreen::callback(index, value, action);
    }
  }
  return true;
}

/**
 * @brief Callback function for handling speed value updates.
 *
 * This function is called when the speed value is updated. It checks the
 * current screen type and calls the appropriate function to handle the update.
 *
 * @param index The index of the speed value.
 * @param value The new value of the speed.
 * @param action The action that triggered the callback.
 * @return true if the callback was handled successfully, false otherwise.
 */
bool speed_value_cb(ESP3DValuesIndex index, const char *value,
                    ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::main) {
      main_display_speed();
    } else {
      speedScreen::callback(index, value, action);
    }
  }
  return true;
}
/**
 * @brief Callback function for handling job status values.
 *
 * This function is called when there is an update in the job status values.
 * It updates the progression area string with the file name and job progress
 * percentage. It also calculates and displays the time elapsed since the job
 * started.
 *
 * @param index The index of the ESP3D value being updated.
 * @param value The new value of the ESP3D value.
 * @param action The action performed on the ESP3D value (e.g., update).
 * @return true if the callback was successfully handled, false otherwise.
 */
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

/**
 * Displays the information for extruder 0 on the main screen.
 */
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

/**
 * @brief Updates the main display for extruder 1.
 *
 * This function updates the main display for extruder 1 with the current
 * temperature and target temperature values. If the target temperature is not
 * available (indicated by "#"), the button for extruder 1 is hidden.
 *
 * @return void
 */
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

/**
 * @brief Displays the bed temperature and target temperature on the main
 * screen.
 *
 * This function retrieves the bed temperature and target temperature values
 * from the ESP3D TFT values, and displays them on the main screen. If the bed
 * temperature is 0, it displays the symbol for no heat bed, otherwise it
 * displays the symbol for heat bed.
 *
 * @note This function assumes that the main_btn_bed object has been initialized
 * and is valid.
 */
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

/**
 * Updates the main display positions with the current X, Y, and Z coordinates.
 */
void main_display_positions() {
  lv_label_set_text_fmt(
      lv_obj_get_child(main_btn_positions, 0), "X: %s\nY: %s\nZ: %s",
      esp3dTftValues.get_string_value(ESP3DValuesIndex::position_x),
      esp3dTftValues.get_string_value(ESP3DValuesIndex::position_y),
      esp3dTftValues.get_string_value(ESP3DValuesIndex::position_z));
}

/**
 * Updates the main status area on the display.
 * If the `main_label_progression_area` is not null, it sets the text of the
 * label to the value of `progression_area_str`.
 */
void main_display_status_area() {
  if (main_label_progression_area)
    lv_label_set_text(main_label_progression_area,
                      progression_area_str.c_str());
}

/**
 * Updates the fan button on the main screen with the fan control information.
 * If there is no control to show, the function returns early.
 * If there are two fans, the button is updated with the fan percentages and the
 * fan symbol. If there is only one fan, the button is updated with the fan
 * percentage and the fan symbol.
 */
void main_display_fan() {
  esp3d_log("main_display_fan");
  if (!show_fan_button) {
    esp3d_log("No control to show");
    return;
  }
  if (nb_fans == 2) {
    esp3d_log("Update button with 2 fans");
    lv_label_set_text_fmt(
        lv_obj_get_child(main_btn_fan, 0), "%s%%\n%s%%\n%s",
        esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_fan),

        esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_1_fan),
        LV_SYMBOL_FAN);
  } else {
    esp3d_log("Update button with 1 fan");
    lv_label_set_text_fmt(

        lv_obj_get_child(main_btn_fan, 0), "%s%%\n%s",
        esp3dTftValues.get_string_value(ESP3DValuesIndex::ext_0_fan),
        LV_SYMBOL_FAN);
  }
}

/**
 * Updates the main screen display with the current speed value.
 * The speed value is retrieved from `esp3dTftValues` and displayed as a
 * percentage. Additionally, the speed symbol is displayed next to the value.
 */
void main_display_speed() {
  lv_label_set_text_fmt(
      lv_obj_get_child(main_btn_speed, 0), "%s%%\n%s",
      esp3dTftValues.get_string_value(ESP3DValuesIndex::speed),
      LV_SYMBOL_SPEED);
}

/**
 * Function to handle the main display pause functionality.
 * It checks the current job status and updates the visibility of the pause
 * button accordingly.
 */
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

/**
 * Resumes the main display based on the current job status.
 * If the job status is "paused", the resume button is shown.
 * If the job status is "processing", the resume button is hidden.
 * For any other job status, the resume button is hidden.
 */
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

/**
 * Stops the main display based on the current job status.
 * If the job status is "paused" or "processing", the stop button is shown.
 * Otherwise, the stop button is hidden.
 */
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

/**
 * Updates the visibility of the main files button based on the job status.
 * If the job status is "paused" or "processing", the button is hidden.
 * Otherwise, the button is shown.
 */
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

/**
 * Function to handle the main display menu.
 * This function checks the job status and updates the visibility of the main
 * menu button accordingly.
 */
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

/**
 * @brief Callback function for the main screen delay timer.
 *
 * This function is called when the main screen delay timer expires. It is
 * responsible for handling the next screen to be displayed based on the value
 * of the next_screen variable.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void main_screen_delay_timer_cb(lv_timer_t *timer) {
  // If timer is not null, delete it to avoid multiple call
  if (main_screen_delay_timer && lv_timer_is_valid(main_screen_delay_timer)) {
    lv_timer_del(main_screen_delay_timer);
  }
  main_screen_delay_timer = NULL;
  switch (next_screen) {
    case ESP3DScreenType::none:
      emptyScreen::create();
      break;
    case ESP3DScreenType::temperatures:
      temperaturesScreen::create(main_screen_temperature_target);
      break;
    case ESP3DScreenType::positions:
      positionsScreen::create();
      break;
    case ESP3DScreenType::fan:
      fanScreen::create();
      break;
    case ESP3DScreenType::speed:
      speedScreen::create();
      break;
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

/**
 * Event handler for the E0 button click event.
 * This function is called when the E0 button is clicked.
 * It sets the next screen to ESP3DScreenType::temperatures,
 * resets the main screen temperature target to 0,
 * and starts a delay timer for button animation if ESP3D_BUTTON_ANIMATION_DELAY
 * is set. If the delay timer is not set, it immediately calls the
 * main_screen_delay_timer_cb function.
 *
 * @param e The event object.
 */
void event_button_E0_handler(lv_event_t *e) {
  esp3d_log("E0 Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_screen_temperature_target = 0;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(
        main_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

/**
 * @brief Event handler for the E1 button click.
 *
 * This function is called when the E1 button is clicked. It logs a message,
 * sets the next screen to temperatures, sets the temperature target to 1, and
 * creates a delay timer for button animation if necessary.
 *
 * @param e The event object.
 */
void event_button_E1_handler(lv_event_t *e) {
  esp3d_log("E1 Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_screen_temperature_target = 1;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(
        main_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

/**
 * @brief Event handler for the "Bed" button click event.
 * This function is called when the "Bed" button is clicked.
 * It sets the next screen to ESP3DScreenType::temperatures and the main screen
 * temperature target to 2. If ESP3D_BUTTON_ANIMATION_DELAY is defined, it
 * creates a delay timer for button animation. If the delay timer is already
 * running, the function returns without performing any action.
 * @param e The event object.
 */
void event_button_Bed_handler(lv_event_t *e) {
  esp3d_log("Bed Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::temperatures;
  main_screen_temperature_target = 2;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(
        main_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

/**
 * @brief Event handler for the "Positions" button click.
 *
 * This function is called when the "Positions" button is clicked on the main
 * screen. It logs a message indicating that the button has been clicked and
 * sets the next screen to be displayed as the "positions" screen. If the button
 * animation delay is enabled, it creates a timer to delay the screen
 * transition. Otherwise, it immediately calls the callback function for the
 * screen transition.
 *
 * @param e Pointer to the event object.
 */
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

/**
 * @brief Event handler for the fan button click event.
 *
 * This function is called when the fan button is clicked on the main screen.
 * It logs a message indicating that the fan button has been clicked.
 * If there is a delay timer active, the function returns without performing any
 * further actions. Otherwise, it sets the next screen to be displayed as the
 * fan screen. If there is a button animation delay configured, it creates a
 * delay timer and sets it to trigger the main_screen_delay_timer_cb function
 * after the specified delay. If there is no button animation delay configured,
 * it directly calls the main_screen_delay_timer_cb function.
 *
 * @param e Pointer to the event object.
 */
void event_button_fan_handler(lv_event_t *e) {
  esp3d_log("Fan Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::fan;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(
        main_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

/**
 * @brief Event handler for the speed button click event.
 *
 * This function is called when the speed button is clicked on the main screen.
 * It logs a message indicating that the speed button has been clicked.
 * If there is a delay timer active, the function returns without performing any
 * further actions. Otherwise, it sets the next screen to be displayed as the
 * speed screen. If button animation delay is enabled, it creates a delay timer
 * to trigger the animation after a certain delay. If button animation delay is
 * disabled, it directly calls the delay timer callback function.
 *
 * @param e Pointer to the event object.
 */
void event_button_speed_handler(lv_event_t *e) {
  esp3d_log("Speed Clicked");
  if (main_screen_delay_timer) return;
  next_screen = ESP3DScreenType::speed;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (main_screen_delay_timer) return;
    main_screen_delay_timer = lv_timer_create(
        main_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    main_screen_delay_timer_cb(NULL);
  }
}

#if ESP3D_SD_CARD_FEATURE
/**
 *@brief Event handler for the "Files" button click event.
 * This function is called when the "Files" button is clicked on the main
 *screen. It logs a message, sets the next screen to the "files" screen, and
 *handles the button animation delay if enabled.
 *
 * @param e The event object associated with the button click event.
 */
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

/**
 * @brief Event handler for the menu button click event.
 * This function is called when the menu button is clicked.
 * It sets the next screen to be displayed as the menu screen and starts a delay
 * timer for button animation. If the delay timer is already running, the
 * function returns without performing any action. If the delay timer is not
 * enabled, the function immediately calls the delay timer callback function.
 *
 * @param e The event object associated with the button click event.
 */
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

/**
 * Event handler for the resume button click event.
 * This function is called when the resume button is clicked on the main screen.
 * It logs a message and calls the `resume` function of the `gcodeHostService`.
 *
 * @param e The event object associated with the button click event.
 */
void event_button_resume_handler(lv_event_t *e) {
  esp3d_log("Resume Clicked");
  gcodeHostService.resume();
}

/**
 * @brief Event handler for the pause button click event.
 *
 * This function is called when the pause button is clicked on the main screen.
 * It logs a message indicating that the pause button was clicked and calls the
 * `pause` function of the `gcodeHostService` object.
 *
 * @param e Pointer to the event object.
 */
void event_button_pause_handler(lv_event_t *e) {
  esp3d_log("Pause Clicked");
  gcodeHostService.pause();
}

/**
 * @brief Callback function for the "confirm stop" event.
 * This function is called when the user confirms the stop action on a message
 * box. It retrieves the button selection and performs the corresponding action.
 *
 * @param e The event object.
 */
void event_confirm_stop_cb(lv_event_t *e) {
  lv_obj_t *mbox = lv_event_get_current_target(e);
  std::string rep = lv_msgbox_get_active_btn_text(mbox);
  esp3d_log("Button selectionned : %s", rep == LV_SYMBOL_OK ? "Ok" : "Cancel");
  if (rep == LV_SYMBOL_OK) {
    gcodeHostService.abort();
  }
  lv_msgbox_close(mbox);
}

/**
 * @brief Handles the event when the stop button is clicked.
 *
 * This function logs a message indicating that the stop button has been
 * clicked. It also retrieves a translated text for the stop print label and
 * displays a confirmation message box.
 *
 * @param e The event object associated with the button click event.
 */
void event_button_stop_handler(lv_event_t *e) {
  esp3d_log("Stop Clicked");
  std::string text = esp3dTranslationService.translate(ESP3DLabel::stop_print);
  msgBox::confirmation(NULL, MsgBoxType::confirmation, text.c_str(),
                       event_confirm_stop_cb);
}

/**
 * @brief Creates the main screen.
 *
 * This function is responsible for creating the main screen of the application.
 * It sets up the user interface elements, such as buttons and labels, and
 * initializes their event handlers. The main screen consists of multiple
 * containers for organizing the buttons and labels. It also retrieves settings
 * from the JSON file and determines whether to show the fan button based on the
 * "showfanctrls" setting. After creating the screen, it sets it as the current
 * screen and displays it to the user. Finally, it calls various functions to
 * update the display of different elements on the screen.
 */
void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Main screen creation");
  if (!intialization_done) {
    esp3d_log("main screen initialization");
    std::string value =
        esp3dTftJsonSettings.readString("settings", "showfanctrls");
    if (value == "true") {
      show_fan_button = true;
    } else {
      show_fan_button = false;
    }
    intialization_done = true;
  }
  // Background
  lv_obj_t *ui_main_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_main_screen)) {
    esp3d_log_e("Error creating main screen");
    return;
  }
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_main_screen);
  ESP3DStyle::apply(ui_main_screen, ESP3DStyleType::main_bg);
  if (lv_obj_is_valid(ui_current_screen)) {
    lv_obj_del(ui_current_screen);
  }

  lv_obj_t *ui_status_bar_container = statusBar::create(ui_main_screen);
  if (!lv_obj_is_valid(ui_status_bar_container)) {
    esp3d_log_e("Error creating status bar");
    return;
  }
  lv_obj_update_layout(ui_status_bar_container);

  // create container for main screen buttons
  lv_obj_t *ui_container_main_screen = lv_obj_create(ui_main_screen);
  if (!lv_obj_is_valid(ui_container_main_screen)) {
    esp3d_log_e("Error creating main screen container");
    return;
  }
  ESP3DStyle::apply(ui_container_main_screen, ESP3DStyleType::col_container);
  lv_obj_clear_flag(ui_container_main_screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(ui_container_main_screen, LV_HOR_RES,
                  LV_VER_RES - lv_obj_get_height(ui_status_bar_container));
  lv_obj_align_to(ui_container_main_screen, ui_status_bar_container,
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  // Add buttons top container to main container
  lv_obj_t *ui_top_buttons_container = lv_obj_create(ui_container_main_screen);
  if (!lv_obj_is_valid(ui_top_buttons_container)) {
    esp3d_log_e("Error creating top buttons container");
    return;
  }
  ESP3DStyle::apply(ui_top_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_top_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_top_buttons_container);
  lv_obj_clear_flag(ui_top_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Middle container
  lv_obj_t *ui_middle_container = lv_btn_create(ui_container_main_screen);
  if (!lv_obj_is_valid(ui_middle_container)) {
    esp3d_log_e("Error creating middle container");
    return;
  }
  ESP3DStyle::apply(ui_middle_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_middle_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_middle_container);

  // Add buttons bottom container to main container
  lv_obj_t *ui_bottom_buttons_container =
      lv_obj_create(ui_container_main_screen);
  if (!lv_obj_is_valid(ui_bottom_buttons_container)) {
    esp3d_log_e("Error creating bottom buttons container");
    return;
  }
  ESP3DStyle::apply(ui_bottom_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_bottom_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_bottom_buttons_container);
  lv_obj_clear_flag(ui_bottom_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  //**********************************
  // Create button and label for Extruder 0
  main_btn_extruder_0 = menuButton::create(ui_top_buttons_container, "");
  if (!lv_obj_is_valid(main_btn_extruder_0)) {
    esp3d_log_e("Error creating extruder 0 button");
    return;
  }

  lv_obj_add_event_cb(main_btn_extruder_0, event_button_E0_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for Extruder 1
  main_btn_extruder_1 = menuButton::create(ui_top_buttons_container, "");
  if (!lv_obj_is_valid(main_btn_extruder_1)) {
    esp3d_log_e("Error creating extruder 1 button");
    return;
  }
  lv_obj_add_event_cb(main_btn_extruder_1, event_button_E1_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for Bed
  main_btn_bed = menuButton::create(ui_top_buttons_container, "");
  if (!lv_obj_is_valid(main_btn_bed)) {
    esp3d_log_e("Error creating bed button");
    return;
  }
  lv_obj_add_event_cb(main_btn_bed, event_button_Bed_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for positions
  main_btn_positions =
      symbolButton::create(ui_top_buttons_container, "",
                           ESP3D_BUTTON_WIDTH * 1.5, ESP3D_BUTTON_HEIGHT);
  if (!lv_obj_is_valid(main_btn_positions)) {
    esp3d_log_e("Error creating positions button");
    return;
  }

  lv_obj_add_event_cb(main_btn_positions, event_button_positions_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for middle container
  main_label_progression_area = lv_label_create(ui_middle_container);
  if (!lv_obj_is_valid(main_label_progression_area)) {
    esp3d_log_e("Error creating progression area label");
    return;
  }
  ESP3DStyle::apply(main_label_progression_area,
                    ESP3DStyleType::progression_area);

  lv_obj_center(main_label_progression_area);

  if (show_fan_button) {
    // Create button and label for fan
    main_btn_fan = menuButton::create(ui_bottom_buttons_container, "");
    if (!lv_obj_is_valid(main_btn_fan)) {
      esp3d_log_e("Error creating fan button");
      return;
    }
    lv_obj_add_event_cb(main_btn_fan, event_button_fan_handler,
                        LV_EVENT_CLICKED, NULL);
  }

  // Create button and label for speed
  main_btn_speed = menuButton::create(ui_bottom_buttons_container, "");
  if (!lv_obj_is_valid(main_btn_speed)) {
    esp3d_log_e("Error creating speed button");
    return;
  }
  lv_obj_add_event_cb(main_btn_speed, event_button_speed_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for pause
  main_btn_pause =
      menuButton::create(ui_bottom_buttons_container, LV_SYMBOL_PAUSE);
  lv_obj_add_event_cb(main_btn_pause, event_button_pause_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for resume
  main_btn_resume =
      menuButton::create(ui_bottom_buttons_container, LV_SYMBOL_PLAY);
  if (!lv_obj_is_valid(main_btn_resume)) {
    esp3d_log_e("Error creating resume button");
    return;
  }

  lv_obj_add_event_cb(main_btn_resume, event_button_resume_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for stop
  main_btn_stop =
      menuButton::create(ui_bottom_buttons_container, LV_SYMBOL_STOP);
  if (!lv_obj_is_valid(main_btn_stop)) {
    esp3d_log_e("Error creating stop button");
    return;
  }
  lv_obj_add_event_cb(main_btn_stop, event_button_stop_handler,
                      LV_EVENT_CLICKED, NULL);
#if ESP3D_SD_CARD_FEATURE
  // Create button and label for files
  main_btn_files =
      menuButton::create(ui_bottom_buttons_container, LV_SYMBOL_SD_CARD);
  if (!lv_obj_is_valid(main_btn_files)) {
    esp3d_log_e("Error creating files button");
    return;
  }
  lv_obj_add_event_cb(main_btn_files, event_button_files_handler,
                      LV_EVENT_CLICKED, NULL);
#endif  // ESP3D_SD_CARD_FEATURE
  // Create button and label for menu
  std::string label_text8 = LV_SYMBOL_LIST;
  main_btn_menu =
      menuButton::create(ui_bottom_buttons_container, LV_SYMBOL_LIST);
  if (!lv_obj_is_valid(main_btn_menu)) {
    esp3d_log_e("Error creating menu button");
    return;
  }
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
#if ESP3D_SD_CARD_FEATURE
  main_display_files();
#endif  // ESP3D_SD_CARD_FEATURE
  main_display_stop();
  main_display_menu();
  main_display_speed();
  if (show_fan_button) main_display_fan();
}
}  // namespace mainScreen