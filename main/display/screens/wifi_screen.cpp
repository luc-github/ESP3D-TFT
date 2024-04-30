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

#include "screens/ap_screen.h"
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
#include "screens/menu_screen.h"
#include "network/esp3d_network.h"
#include "screens/sta_screen.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace wifiScreen {
lv_timer_t *wifi_screen_delay_timer = NULL;
lv_obj_t *btnback = nullptr;
#if ESP3D_PATCH_DELAY_REFRESH
lv_timer_t *wifi_screen_delay_refresh_timer = NULL;
void wifi_screen_delay_refresh_timer_cb(lv_timer_t *timer) {
  esp3d_log("wifi_screen_delay_refresh_timer_cb");
  if (wifi_screen_delay_refresh_timer) {
    lv_timer_del(wifi_screen_delay_refresh_timer);
    wifi_screen_delay_refresh_timer = NULL;
  }
  spinnerScreen::hide();
}
#endif  // ESP3D_PATCH_DELAY_REFRESH
lv_timer_t *wifi_screen_delay_connecting_timer = nullptr;
ESP3DScreenType wifi_next_screen = ESP3DScreenType::none;
void wifi_screen_delay_timer_cb(lv_timer_t *timer);
void wifi_screen_delay_connecting_timer_cb(lv_timer_t *timer) {
  esp3d_log("wifi_screen_delay_connecting_timer_cb");
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::wifi) {
    if (wifi_screen_delay_connecting_timer) {
      lv_timer_del(wifi_screen_delay_connecting_timer);
      wifi_screen_delay_connecting_timer = NULL;
    }
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
    if (wifi_screen_delay_connecting_timer) {
      lv_timer_del(wifi_screen_delay_connecting_timer);
      wifi_screen_delay_connecting_timer = NULL;
    }
    spinnerScreen::hide();
    wifi_next_screen = ESP3DScreenType::wifi;
    wifi_screen_delay_timer =
        lv_timer_create(wifi_screen_delay_timer_cb, 50, NULL);
  }
}

lv_obj_t *btn_no_wifi = nullptr;

void update_button_no_wifi() {
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

void wifi_screen_delay_timer_cb(lv_timer_t *timer) {
  esp3d_log("wifi_screen_delay_timer_cb");
  if (wifi_screen_delay_timer) {
    lv_timer_del(wifi_screen_delay_timer);
    wifi_screen_delay_timer = NULL;
  }
  if (wifi_screen_delay_connecting_timer) {
    lv_timer_del(wifi_screen_delay_connecting_timer);
    wifi_screen_delay_connecting_timer = NULL;
  }
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
      wifiScreen::wifi_screen();
      break;
    default:
      break;
  }
}

void event_button_wifi_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::menu;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(wifi_screen_delay_timer_cb,
                                              ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}

void event_button_ap_handler(lv_event_t *e) {
  esp3d_log("AP Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::access_point;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(wifi_screen_delay_timer_cb,
                                              ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}

void event_button_sta_handler(lv_event_t *e) {
  esp3d_log("STA Clicked");
  if (wifi_screen_delay_timer) return;
  wifi_next_screen = ESP3DScreenType::station;
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (wifi_screen_delay_timer) return;
    wifi_screen_delay_timer = lv_timer_create(wifi_screen_delay_timer_cb,
                                              ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    wifi_screen_delay_timer_cb(NULL);
  }
}
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

void wifi_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Wifi screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_del(ui_current_screen);

  btnback = backButton::create(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_wifi_back_handler, LV_EVENT_CLICKED,
                      NULL);
  wifiStatus::create(ui_new_screen, btnback);
  lv_obj_t *ui_main_container = mainContainer::create(
      ui_new_screen, btnback, ESP3DStyleType::col_container);

  lv_obj_t *ui_buttons_container = lv_obj_create(ui_main_container);
  ESP3DStyle::apply(ui_buttons_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_buttons_container, LV_HOR_RES, LV_SIZE_CONTENT);
  ESP3DStyle::add_pad(ui_buttons_container);
  lv_obj_clear_flag(ui_buttons_container, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *btn = nullptr;

  // Create button and label for ap
  btn = symbolButton::create(
      ui_buttons_container, LV_SYMBOL_ACCESS_POINT, ESP3D_BUTTON_WIDTH, ESP3D_BUTTON_WIDTH);
  lv_obj_add_event_cb(btn, event_button_ap_handler, LV_EVENT_CLICKED, NULL);

  // Create button and label for sta
  btn = symbolButton::create(
      ui_buttons_container, LV_SYMBOL_STATION_MODE, ESP3D_BUTTON_WIDTH, ESP3D_BUTTON_WIDTH);
  lv_obj_add_event_cb(btn, event_button_sta_handler, LV_EVENT_CLICKED, NULL);

  // Create button and label for no wifi
  btn_no_wifi = symbolButton::create(ui_buttons_container,
                                                   LV_SYMBOL_WIFI, ESP3D_BUTTON_WIDTH,
                                                   ESP3D_BUTTON_WIDTH, true, true);
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
    update_button_no_wifi();
  }
}
}  // namespace wifiScreen
#endif  // ESP3D_WIFI_FEATURE