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
#if ESP3D_WIFI_FEATURE
#include "screens/wifi_screen.h"

#include <lvgl.h>

#include "bsp.h"
#include "components/back_button_component.h"
#include "components/main_container_component.h"
#include "components/message_box_component.h"
#include "components/spinner_component.h"
#include "components/symbol_button_component.h"
#include "components/wifi_status_component.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "esp3d_lvgl.h"
#include "network/esp3d_network.h"
#include "screens/ap_screen.h"
#include "screens/menu_screen.h"
#include "screens/sta_screen.h"
#include "translations/esp3d_translation_service.h"


/**********************
 * Namespace
 **********************/
namespace wifiScreen {
// Static variables
lv_timer_t *wifi_screen_delay_timer = NULL;
lv_obj_t *btnback = nullptr;
lv_timer_t *wifi_screen_delay_connecting_timer = nullptr;
ESP3DScreenType wifi_next_screen = ESP3DScreenType::none;
lv_obj_t *btn_no_wifi = nullptr;

#if ESP3D_PATCH_DELAY_REFRESH
lv_timer_t *wifi_screen_delay_refresh_timer = NULL;

/**
 * @brief Callback function for the WiFi screen delay refresh timer.
 *
 * This function is called when the delay refresh timer for the WiFi screen expires.
 * It is responsible for deleting the timer and hiding the spinner screen.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void wifi_screen_delay_refresh_timer_cb(lv_timer_t *timer) {
  esp3d_log("wifi_screen_delay_refresh_timer_cb");
  if (wifi_screen_delay_refresh_timer) {
    lv_timer_del(wifi_screen_delay_refresh_timer);
    wifi_screen_delay_refresh_timer = NULL;
  }
  spinnerScreen::hide();
}
#endif  // ESP3D_PATCH_DELAY_REFRESH

// Static functions prototypes
void wifi_screen_delay_timer_cb(lv_timer_t *timer);

// Static functions

/**
 * @brief Callback function for the delay connecting timer in the WiFi screen.
 *
 * This function is called when the delay connecting timer expires. It checks the current screen and network status to determine the next action.
 * If the current screen is not the WiFi screen or the network status is not "x", the timer is deleted and the function returns.
 * Otherwise, the spinner screen is hidden, the next screen is set to the WiFi screen, and a new delay timer is created.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void wifi_screen_delay_connecting_timer_cb(lv_timer_t *timer) {
  esp3d_log("wifi_screen_delay_connecting_timer_cb");
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::wifi) {
    if (wifi_screen_delay_connecting_timer && lv_timer_is_valid(wifi_screen_delay_connecting_timer)) {
      lv_timer_del(wifi_screen_delay_connecting_timer);
    }
    wifi_screen_delay_connecting_timer = NULL;
    return;
  }
  std::string mode =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_mode);
  std::string status =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_status);
  esp3d_log("mode: %s, %s,  status: %s", mode.c_str(),
            mode == LV_SYMBOL_STATION_MODE   ? "STA"
            : mode == LV_SYMBOL_ACCESS_POINT ? "AP"
            : mode == LV_SYMBOL_WIFI         ? "NO WIFI"
                                             : "?",
            status.c_str());
  if (mode == LV_SYMBOL_WIFI && status == "x") {
  } else {
    if (wifi_screen_delay_connecting_timer && lv_timer_is_valid(wifi_screen_delay_connecting_timer)) {
      lv_timer_del(wifi_screen_delay_connecting_timer);  
    }
    wifi_screen_delay_connecting_timer = NULL;
    spinnerScreen::hide();
    wifi_next_screen = ESP3DScreenType::wifi;
    wifi_screen_delay_timer =
        lv_timer_create(wifi_screen_delay_timer_cb, 50, NULL);
    if (!wifi_screen_delay_timer) {
      esp3d_log_e("Failed to create delay timer");
    }
  }
}

/**
 * @brief Callback function for handling updates on the WiFi screen.
 * 
 * This function is called when there is a need to update the WiFi screen. It checks if the current screen is the WiFi screen and performs the necessary updates based on the network mode. If the network mode is set to LV_SYMBOL_WIFI, the "No WiFi" button is hidden. Otherwise, the "No WiFi" button is shown.
 */
void callback() {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::wifi) return;
  esp3d_log("update_button_no_wifi");
  std::string mode =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_mode);
  if (mode == LV_SYMBOL_WIFI) {
    lv_obj_add_flag(btn_no_wifi, LV_OBJ_FLAG_HIDDEN);
#if ESP3D_PATCH_DELAY_REFRESH
    if (wifi_screen_delay_refresh_timer) {
      lv_timer_del(wifi_screen_delay_refresh_timer);
      wifi_screen_delay_refresh_timer = NULL;
    }
    wifi_screen_delay_refresh_timer =
        lv_timer_create(wifi_screen_delay_refresh_timer_cb, 100, NULL);
#else
    spinnerScreen::hide();
#endif  // ESP3D_PATCH_DELAY_REFRESH
  } else {
    lv_obj_clear_flag(btn_no_wifi, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * @brief Callback function for the WiFi screen delay timer.
 *
 * This function is called when the WiFi screen delay timer expires. It performs the necessary cleanup
 * and navigation logic based on the next screen to be displayed.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void wifi_screen_delay_timer_cb(lv_timer_t *timer) {
  esp3d_log("wifi_screen_delay_timer_cb");
  if (wifi_screen_delay_timer && lv_timer_is_valid(wifi_screen_delay_timer)) {
    lv_timer_del(wifi_screen_delay_timer); 
  } 
  wifi_screen_delay_timer = NULL;
  if (wifi_screen_delay_connecting_timer  && lv_timer_is_valid(wifi_screen_delay_connecting_timer)) {
    lv_timer_del(wifi_screen_delay_connecting_timer);
  } 
  wifi_screen_delay_connecting_timer = NULL;
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
    case ESP3DScreenType::wifi:
      wifiScreen::create();
      break;
    default:
      break;
  }
}

/**
 * Event handler for the "back" button in the WiFi screen.
 * This function is called when the "back" button is clicked.
 * It logs a message, sets the next screen to the menu screen, and starts a timer for button animation delay if enabled.
 * If the button animation delay is not enabled, it directly calls the callback function.
 *
 * @param e The event object associated with the button click event.
 */
void event_button_wifi_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::menu;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(
        wifi_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}

/**
 * @brief Event handler for the AP button click event.
 *
 * This function is called when the AP button is clicked on the WiFi screen.
 * It logs a message, sets the next screen to the access point screen, and
 * creates a timer to delay the screen transition if necessary.
 *
 * @param e Pointer to the event object.
 */
void event_button_ap_handler(lv_event_t *e) {
  esp3d_log("AP Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::access_point;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(
        wifi_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}

/**
 * @brief Event handler for the STA button click event.
 *
 * This function is called when the STA button is clicked on the WiFi screen.
 * It logs a message, sets the next screen to the station screen, and creates a timer
 * to delay the screen transition if animation delay is enabled.
 *
 * @param e Pointer to the event object.
 */
void event_button_sta_handler(lv_event_t *e) {
  esp3d_log("STA Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::station;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(
        wifi_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}

/**
 * Event handler for the "no wifi" button click event.
 * This function is called when the "no wifi" button is clicked.
 * It sets the ESP3D radio mode to "off" and updates the UI accordingly.
 * If the mode change fails, an error message is displayed.
 * After setting the mode, a spinner screen is shown and the network mode is updated asynchronously.
 *
 * @param e The event object associated with the button click event.
 */
void event_button_no_wifi_handler(lv_event_t *e) {
  esp3d_log("no wifi Clicked");
  if (!esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_radio_mode,
                                  static_cast<uint8_t>(ESP3DRadioMode::off))) {
    std::string text =
        esp3dTranslationService.translate(ESP3DLabel::error_applying_mode);
    msgBox::create(NULL, MsgBoxType::error, text.c_str());
    esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                    text.c_str());
  } else {
    spinnerScreen::show();
    esp3dNetwork.setModeAsync(ESP3DRadioMode::off);
  }
}

/**
 * @brief Creates the WiFi screen.
 * 
 * This function is responsible for creating the WiFi screen in the ESP3D-TFT application.
 * It sets the current screen to none, creates the UI elements for the WiFi screen,
 * and handles the logic for connecting to WiFi.
 * 
 * @note This function assumes that the necessary UI elements and styles have been defined.
 * 
 * @return void
 */
void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Wifi screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_new_screen)) {
    esp3d_log_e("Failed to create wifi screen");
    return;
  }
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  if (lv_obj_is_valid(ui_current_screen)){
    lv_obj_del(ui_current_screen);
  }
  btnback = backButton::create(ui_new_screen);
  if (!lv_obj_is_valid(btnback)) {
    esp3d_log_e("Failed to create back button");
    return;
  }
  lv_obj_add_event_cb(btnback, event_button_wifi_back_handler, LV_EVENT_CLICKED,
                      NULL);
  wifiStatus::create(ui_new_screen, btnback);
  lv_obj_t *ui_main_container = mainContainer::create(
      ui_new_screen, btnback, ESP3DStyleType::col_container);
  if (!lv_obj_is_valid(ui_main_container)) {
    esp3d_log_e("Failed to create main container");
    return;
  }
  lv_obj_t *ui_buttons_container = lv_obj_create(ui_main_container);
  if (!lv_obj_is_valid(ui_buttons_container)) {
    esp3d_log_e("Failed to create buttons container");
    return;
  }
  ESP3DStyle::apply(ui_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_buttons_container);
  lv_obj_clear_flag(ui_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *btn = nullptr;

  // Create button and label for ap
  btn = symbolButton::create(ui_buttons_container, LV_SYMBOL_ACCESS_POINT,
                             ESP3D_BUTTON_WIDTH, ESP3D_BUTTON_WIDTH);
  lv_obj_add_event_cb(btn, event_button_ap_handler, LV_EVENT_CLICKED, NULL);

  // Create button and label for sta
  btn = symbolButton::create(ui_buttons_container, LV_SYMBOL_STATION_MODE,
                             ESP3D_BUTTON_WIDTH, ESP3D_BUTTON_WIDTH);
  lv_obj_add_event_cb(btn, event_button_sta_handler, LV_EVENT_CLICKED, NULL);

  // Create button and label for no wifi
  btn_no_wifi =
      symbolButton::create(ui_buttons_container, LV_SYMBOL_WIFI,
                           ESP3D_BUTTON_WIDTH, ESP3D_BUTTON_WIDTH, true, true);
  lv_obj_add_event_cb(btn_no_wifi, event_button_no_wifi_handler,
                      LV_EVENT_CLICKED, NULL);

  esp3dTftui.set_current_screen(ESP3DScreenType::wifi);
  std::string mode =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_mode);
  std::string status =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_status);
  esp3d_log("mode: %s, %s,  status: %s", mode.c_str(),
            mode == LV_SYMBOL_STATION_MODE   ? "STA"
            : mode == LV_SYMBOL_ACCESS_POINT ? "AP"
            : mode == LV_SYMBOL_WIFI         ? "NO WIFI"
                                             : "?",
            status.c_str());
  if (mode == LV_SYMBOL_WIFI && status == "x") {
    std::string text =
        esp3dTranslationService.translate(ESP3DLabel::connecting);
    spinnerScreen::show(text.c_str(), btnback);
    wifi_screen_delay_connecting_timer =
        lv_timer_create(wifi_screen_delay_connecting_timer_cb, 500, NULL);
  } else {
    callback();
  }
}
}  // namespace wifiScreen
#endif  // ESP3D_WIFI_FEATURE