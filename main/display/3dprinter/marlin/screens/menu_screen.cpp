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
#include "esp3d_lvgl.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "esp3d_values.h"
#include "rendering/esp3d_rendering_client.h"
#include "screens/filament_screen.h"
#include "screens/informations_screen.h"
#include "screens/leveling_screen.h"
#include "screens/main_screen.h"
#include "screens/manual_leveling_screen.h"
#include "screens/menu_screen.h"
#include "screens/settings_screen.h"
#include "translations/esp3d_translation_service.h"

#if ESP3D_WIFI_FEATURE
#include "screens/wifi_screen.h"
#endif  // ESP3D_WIFI_FEATURE

/**********************
 *  Namespace
 **********************/
namespace menuScreen {
// Static variables
lv_timer_t *menu_screen_delay_timer = NULL;
ESP3DScreenType menu_next_screen = ESP3DScreenType::none;
lv_obj_t *main_btn_leveling = NULL;
lv_obj_t *main_btn_disable_steppers = NULL;
bool intialization_done = false;
bool auto_leveling = false;

// Static functions
/**
 * Enables or disables auto leveling.
 *
 * @param enable - A boolean value indicating whether to enable or disable auto
 * leveling.
 */
void enable_auto_leveling(bool enable) { auto_leveling = enable; }

/**
 * Function to handle the display of the leveling menu.
 * It checks the current job status and hides or shows the main leveling button
 * accordingly.
 */
void menu_display_leveling() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::job_status);
  if (label_text == "paused") {
    lv_obj_add_flag(main_btn_leveling, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "processing") {
    lv_obj_add_flag(main_btn_leveling, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(main_btn_leveling, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * Disables the stepper motors on the menu screen.
 * The visibility of the disable steppers button is determined based on the
 * current job status. If the job status is "paused" or "processing", the button
 * is hidden. Otherwise, the button is visible.
 */
void menu_display_disable_steppers() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::job_status);
  if (label_text == "paused") {
    lv_obj_add_flag(main_btn_disable_steppers, LV_OBJ_FLAG_HIDDEN);
  } else if (label_text == "processing") {
    lv_obj_add_flag(main_btn_disable_steppers, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(main_btn_disable_steppers, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * @brief Callback function for updating job status value.
 *
 * This function is called when the job status value is updated. It checks if
 * the current screen is the menu screen and updates the display accordingly.
 *
 * @param index The index of the job status value.
 * @param value The new value of the job status.
 * @param action The action performed on the job status value.
 */
void job_status_value_cb(ESP3DValuesIndex index, const char *value,
                         ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::menu) {
      menu_display_leveling();
      menu_display_disable_steppers();
    }
  }
  // Todo update buttons display according status
}

/**
 * @brief Callback function for the menu screen delay timer.
 *
 * This function is called when the menu screen delay timer expires. It is
 * responsible for switching to the next screen based on the value of
 * `menu_next_screen`.
 *
 * @param timer A pointer to the timer object that triggered the callback.
 */
void menu_screen_delay_timer_cb(lv_timer_t *timer) {
  if (menu_screen_delay_timer && lv_timer_is_valid(menu_screen_delay_timer)) {
    lv_timer_del(menu_screen_delay_timer);
  }
  menu_screen_delay_timer = NULL;
  switch (menu_next_screen) {
    case ESP3DScreenType::main:
      mainScreen::create();
      break;
    case ESP3DScreenType::filament:
      filamentScreen::create();
      break;
    case ESP3DScreenType::settings:
      settingsScreen::create();
      break;
    case ESP3DScreenType::leveling:
      if (auto_leveling) {
        levelingScreen::create(auto_leveling);
      } else {
        manualLevelingScreen::create(auto_leveling);
      }
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

/**
 * @brief Event handler for the "back" button in the menu screen.
 *
 * This function is called when the "back" button is clicked in the menu screen.
 * It logs a message indicating that the button has been clicked, and sets the
 * next screen to be displayed as the main screen. If there is a delay timer
 * set, the function creates a new timer with the specified animation delay. If
 * there is no delay timer set, the function immediately calls the callback
 * function for the delay timer.
 *
 * @param e Pointer to the event object.
 */
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

/**
 * @brief Event handler for the filament button click event.
 *
 * This function is called when the filament button is clicked. It logs a
 * message, sets the next screen to the filament screen, and creates a delay
 * timer for button animation if specified. If the delay is not specified, the
 * delay timer callback is called immediately.
 *
 * @param e The event object associated with the button click event.
 */
void event_button_filament_handler(lv_event_t *e) {
  esp3d_log("filament Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::filament;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(
        menu_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    menu_screen_delay_timer_cb(NULL);
  }
}

#if ESP3D_WIFI_FEATURE
/**
 * @brief Event handler for the WiFi button click event.
 *
 * This function is called when the WiFi button is clicked. It logs a message,
 * sets the next screen to the WiFi screen, and creates a delay timer for button
 * animation if necessary.
 *
 * @param e The event object.
 */
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

/**
 * @brief Event handler for the settings button click event.
 *
 * This function is called when the settings button is clicked on the menu
 * screen. It logs a message indicating that the settings button has been
 * clicked. If there is a delay timer active, the function returns without
 * performing any further actions. Otherwise, it sets the next screen to be
 * displayed as the settings screen. If the button animation delay is enabled,
 * it creates a delay timer and sets the callback function to
 * menu_screen_delay_timer_cb. If the button animation delay is disabled, it
 * directly calls the menu_screen_delay_timer_cb function.
 *
 * @param e Pointer to the event object.
 */
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

/**
 * @brief Event handler for the "leveling" button click event.
 *
 * This function is called when the "leveling" button is clicked on the menu
 * screen. It logs a message, sets the next screen to the leveling screen, and
 * creates a timer for button animation delay if enabled. If the animation delay
 * is not enabled, it immediately calls the menu_screen_delay_timer_cb function.
 *
 * @param e Pointer to the lv_event_t structure containing event data.
 */
void event_button_leveling_handler(lv_event_t *e) {
  esp3d_log("leveling Clicked");
  if (menu_screen_delay_timer) return;
  menu_next_screen = ESP3DScreenType::leveling;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (menu_screen_delay_timer) return;
    menu_screen_delay_timer = lv_timer_create(
        menu_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else
    menu_screen_delay_timer_cb(NULL);
}

/**
 * @brief Event handler for the "informations" button click event.
 *
 * This function is called when the "informations" button is clicked on the menu
 * screen. It logs a message, sets the next screen to be displayed as the
 * "informations" screen, and optionally creates a timer for button animation
 * delay.
 *
 * @param e Pointer to the event object.
 */
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

/**
 * @brief Event handler for the "Disable Steppers" button click.
 *
 * This function is called when the "Disable Steppers" button is clicked on the
 * menu screen. It sends the "M84" G-code command to disable the steppers of the
 * 3D printer. It also displays a message box with the translated text for the
 * "motors_disabled" label. Finally, it updates the status bar label with the
 * translated text.
 *
 * @param e The event object associated with the button click event.
 */
void event_button_disable_steppers_handler(lv_event_t *e) {
  esp3d_log("Disable Steppers Clicked");
  renderingClient.sendGcode("M84");
  std::string text =
      esp3dTranslationService.translate(ESP3DLabel::motors_disabled);
  msgBox::create(NULL, MsgBoxType::information, text.c_str());
  esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                  text.c_str());
}

/**
 * @brief Creates the menu screen.
 *
 * This function initializes and creates the menu screen for the ESP3D TFT UI.
 * It sets the current screen to none if it hasn't been initialized yet.
 * It creates the UI elements such as buttons and containers for the menu
 * screen. The buttons include filament, leveling, disable steppers, wifi,
 * settings, and information buttons. Each button has an associated event
 * handler for handling button clicks. The function also sets the current screen
 * to the menu screen and displays the appropriate UI elements.
 */
void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  if (!intialization_done) {
    esp3d_log("menu screen initialization");
    uint8_t byteValue =
        esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_auto_level_on);

    auto_leveling = byteValue == 1 ? true : false;
    intialization_done = true;
  }
  // Screen creation
  esp3d_log("menu screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_new_screen)) {
    esp3d_log_e("Failed to create menu screen");
    return;
  }
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  if (lv_obj_is_valid(ui_current_screen)) {
    lv_obj_del(ui_current_screen);
  }

  lv_obj_t *btnback = backButton::create(ui_new_screen);
  if (!lv_obj_is_valid(btnback)) {
    esp3d_log_e("Failed to create back button");
    return;
  }
  lv_obj_add_event_cb(btnback, event_button_menu_back_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_t *ui_main_container = mainContainer::create(
      ui_new_screen, btnback, ESP3DStyleType::col_container);
  if (!lv_obj_is_valid(ui_main_container)) {
    esp3d_log_e("Failed to create main container");
    return;
  }
  // Add buttons top container to main container
  lv_obj_t *ui_top_buttons_container = lv_obj_create(ui_main_container);
  if (!lv_obj_is_valid(ui_top_buttons_container)) {
    esp3d_log_e("Failed to create top buttons container");
    return;
  }
  ESP3DStyle::apply(ui_top_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_top_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_top_buttons_container);
  lv_obj_clear_flag(ui_top_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  // Add buttons bottom container to main container
  lv_obj_t *ui_bottom_buttons_container = lv_obj_create(ui_main_container);
  if (!lv_obj_is_valid(ui_bottom_buttons_container)) {
    esp3d_log_e("Failed to create bottom buttons container");
    return;
  }
  ESP3DStyle::apply(ui_bottom_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_bottom_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_bottom_buttons_container);
  lv_obj_clear_flag(ui_bottom_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  //**********************************

  // Create button and label for filament button
  lv_obj_t *btn1 = symbolButton::create(ui_top_buttons_container,
                                        LV_SYMBOL_FILAMENT, ESP3D_BUTTON_WIDTH,
                                        ESP3D_BUTTON_HEIGHT, true, false, 90);
  if (!lv_obj_is_valid(btn1)) {
    esp3d_log_e("Failed to create filament button");
    return;
  }
  lv_obj_add_event_cb(btn1, event_button_filament_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for leveling button
  std::string label2 = LV_SYMBOL_LEVELING;
  main_btn_leveling =
      menuButton::create(ui_top_buttons_container, label2.c_str());
  if (!lv_obj_is_valid(main_btn_leveling)) {
    esp3d_log_e("Failed to create leveling button");
    return;
  }
  lv_obj_add_event_cb(main_btn_leveling, event_button_leveling_handler,
                      LV_EVENT_CLICKED, NULL);

  // Create button and label for disable steppers button
  main_btn_disable_steppers = symbolButton::create(
      ui_top_buttons_container, LV_SYMBOL_ENGINE, ESP3D_BUTTON_WIDTH,
      ESP3D_BUTTON_HEIGHT, true, true, 90);
  if (!lv_obj_is_valid(main_btn_disable_steppers)) {
    esp3d_log_e("Failed to create disable steppers button");
    return;
  }
  lv_obj_add_event_cb(main_btn_disable_steppers,
                      event_button_disable_steppers_handler, LV_EVENT_CLICKED,
                      NULL);

#if ESP3D_WIFI_FEATURE
  // Create button and label for wifi button
  std::string label4 = LV_SYMBOL_WIFI;
  lv_obj_t *btn4 =
      menuButton::create(ui_bottom_buttons_container, label4.c_str());
  if (!lv_obj_is_valid(btn4)) {
    esp3d_log_e("Failed to create wifi button");
    return;
  }
  lv_obj_add_event_cb(btn4, event_button_wifi_handler, LV_EVENT_CLICKED, NULL);
#endif  // ESP3D_WIFI_FEATURE

  // Create button and label for settings button
  std::string label3 = LV_SYMBOL_SETTINGS;
  lv_obj_t *btn3 =
      menuButton::create(ui_bottom_buttons_container, label3.c_str());
  if (!lv_obj_is_valid(btn3)) {
    esp3d_log_e("Failed to create settings button");
    return;
  }
  lv_obj_add_event_cb(btn3, event_button_settings_handler, LV_EVENT_CLICKED,
                      NULL);

  // Create button and label for informations button
  std::string label6 = LV_SYMBOL_MORE_INFO;
  lv_obj_t *btn6 =
      menuButton::create(ui_bottom_buttons_container, label6.c_str());
  if (!lv_obj_is_valid(btn6)) {
    esp3d_log_e("Failed to create informations button");
    return;
  }
  lv_obj_add_event_cb(btn6, event_button_informations_handler, LV_EVENT_CLICKED,
                      NULL);
  esp3dTftui.set_current_screen(ESP3DScreenType::menu);
  menu_display_disable_steppers();
  menu_display_leveling();
}
}  // namespace menuScreen