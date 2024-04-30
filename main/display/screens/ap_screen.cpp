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

#include "screens/ap_screen.h"

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
#include "network/esp3d_network.h"
#include "translations/esp3d_translation_service.h"
#include "screens/wifi_screen.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace apScreen {
lv_timer_t *ap_screen_delay_timer = NULL;

lv_obj_t *ap_ta_ssid = NULL;
lv_obj_t *ap_ta_password = NULL;
lv_obj_t *btn_ok = nullptr;
lv_obj_t *btn_save = nullptr;
#if ESP3D_PATCH_DELAY_REFRESH
lv_timer_t *ap_screen_delay_refresh_timer = NULL;
void ap_screen_delay_refresh_timer_cb(lv_timer_t *timer) {
  esp3d_log("ap_screen_delay_refresh_timer_cb");
  if (ap_screen_delay_refresh_timer) {
    lv_timer_del(ap_screen_delay_refresh_timer);
    ap_screen_delay_refresh_timer = NULL;
  }
  spinnerScreen::hide();
}
#endif  // ESP3D_PATCH_DELAY_REFRESH
std::string ssid_ini;
std::string password_ini;
std::string ssid_current;
std::string password_current;
void update_button_save();

bool save_parameters() {
  bool res = true;
  esp3d_log("Save parameters");
  if (!esp3dTftsettings.writeString(ESP3DSettingIndex::esp3d_ap_ssid,
                                    ssid_current.c_str())) {
    std::string text =
        esp3dTranslationService.translate(ESP3DLabel::error_applying_mode);
    msgBox::create(NULL, MsgBoxType::error, text.c_str());
    esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                    text.c_str());
    res = false;
  } else {
    ssid_ini = ssid_current;
    if (!esp3dTftsettings.writeString(ESP3DSettingIndex::esp3d_ap_password,
                                      password_current.c_str())) {
      std::string text =
          esp3dTranslationService.translate(ESP3DLabel::error_applying_mode);
      msgBox::create(NULL, MsgBoxType::error, text.c_str());
      esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                      text.c_str());
      res = false;
    } else {
      password_ini = password_current;
    }
  }
  update_button_save();
  return res;
}

void update_button_ok() {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::access_point) return;
  esp3d_log("Update ok vibility");
  std::string mode =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_mode);
  if (!esp3dTftsettings.isValidStringSetting(
          ssid_current.c_str(), ESP3DSettingIndex::esp3d_ap_ssid) ||
      (mode == LV_SYMBOL_ACCESS_POINT && ssid_ini == ssid_current) ||
      !esp3dTftsettings.isValidStringSetting(
          password_current.c_str(), ESP3DSettingIndex::esp3d_ap_password)) {
    esp3d_log("Ok hide");
    lv_obj_add_flag(btn_ok, LV_OBJ_FLAG_HIDDEN);
#if ESP3D_PATCH_DELAY_REFRESH
    if (ap_screen_delay_refresh_timer) {
      lv_timer_del(ap_screen_delay_refresh_timer);
      ap_screen_delay_refresh_timer = NULL;
    }
    ap_screen_delay_refresh_timer =
        lv_timer_create(ap_screen_delay_refresh_timer_cb, 100, NULL);
#else
    spinnerScreen::hide();
#endif  // ESP3D_PATCH_DELAY_REFRESH
  } else {
    esp3d_log("Ok visible");
    lv_obj_clear_flag(btn_ok, LV_OBJ_FLAG_HIDDEN);
  }
}

void update_button_save() {
  esp3d_log("Update save vibility");
  if (ssid_ini == ssid_current) {
    lv_obj_add_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
    esp3d_log("Save hide");
    if (password_ini == password_current) {
      lv_obj_add_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
    }
  } else {
    lv_obj_clear_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
    esp3d_log("Save visible");
  }
  if (!esp3dTftsettings.isValidStringSetting(
          ssid_current.c_str(), ESP3DSettingIndex::esp3d_ap_ssid)) {
    lv_obj_add_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
  }
  if (!esp3dTftsettings.isValidStringSetting(
          password_current.c_str(), ESP3DSettingIndex::esp3d_ap_password)) {
    lv_obj_add_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
  }
}

void ap_screen_delay_timer_cb(lv_timer_t *timer) {
  if (ap_screen_delay_timer) {
    lv_timer_del(ap_screen_delay_timer);
    ap_screen_delay_timer = NULL;
  }
  wifiScreen::wifi_screen();
}

void event_button_ap_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  ap_screen_delay_timer =
      lv_timer_create(ap_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
}

void ap_ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_add_state(ta, LV_STATE_FOCUSED);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_AUTO);
    if (ta == ap_ta_password) {
      lv_textarea_set_password_mode(ta, false);
    }
    update_button_ok();
    update_button_save();
  }

  else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_CANCEL) {
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    if (ta == ap_ta_password) {
      lv_textarea_set_password_mode(ta, true);
    }
    update_button_ok();
    update_button_save();
  } else if (code == LV_EVENT_READY) {
    lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    if (ta == ap_ta_ssid) {
      ssid_current = lv_textarea_get_text(ta);
      esp3d_log("Ready, SSID: %s", lv_textarea_get_text(ta));
    } else if (ta == ap_ta_password) {
      esp3d_log("Ready, PASSWORD: %s", lv_textarea_get_text(ta));
      password_current = lv_textarea_get_text(ta);
    }
    update_button_ok();
    update_button_save();
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    if (ta == ap_ta_ssid) {
      ssid_current = lv_textarea_get_text(ta);
      esp3d_log("Value changed, SSID: %s > %s", ssid_ini.c_str(),
                ssid_current.c_str());
    } else if (ta == ap_ta_password) {
      password_current = lv_textarea_get_text(ta);
      esp3d_log("Value changed, PASSWORD: %s > %s", password_ini.c_str(),
                password_current.c_str());
    }
    update_button_ok();
    update_button_save();
  } else {
    // esp3d_log("Unhandled event %d", code);
  }
}

void ap_event_button_ok_handler(lv_event_t *e) {
  esp3d_log("Ok Clicked");
  esp3d_log("ssid: %s, password:%s", ssid_current.c_str(),
            password_current.c_str());
  // Do save first
  if (!save_parameters()) {
    return;
  }

  // Apply now
  if (!esp3dTftsettings.writeByte(
          ESP3DSettingIndex::esp3d_radio_mode,
          static_cast<uint8_t>(ESP3DRadioMode::wifi_ap))) {
    std::string text =
        esp3dTranslationService.translate(ESP3DLabel::error_applying_mode);
    msgBox::create(NULL, MsgBoxType::error, text.c_str());
    esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                    text.c_str());
  } else {
    spinnerScreen::show();
    esp3dNetwork.setModeAsync(ESP3DRadioMode::wifi_ap);
  }
}

void ap_event_button_save_handler(lv_event_t *e) {
  esp3d_log("Save Clicked");
  esp3d_log("ssid: %s, password:%s", ssid_current.c_str(),
            password_current.c_str());
  save_parameters();
}

void ap_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("AP screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_del(ui_current_screen);
  lv_obj_t *btnback = backButton::create(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_ap_back_handler, LV_EVENT_CLICKED,
                      NULL);

  // SSID
  lv_obj_t *label_ssid = lv_label_create(ui_new_screen);
  lv_label_set_text(label_ssid, LV_SYMBOL_ACCESS_POINT);
  ESP3DStyle::apply(label_ssid, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label_ssid);
  int width_label = lv_obj_get_width(label_ssid);
  esp3d_log("width_label %d", lv_obj_get_width(label_ssid));
  ap_ta_ssid = lv_textarea_create(ui_new_screen);
  lv_textarea_set_one_line(ap_ta_ssid, true);
  lv_textarea_set_max_length(ap_ta_ssid, 32);
  lv_obj_align(ap_ta_ssid, LV_ALIGN_TOP_LEFT,
               width_label + (ESP3D_BUTTON_PRESSED_OUTLINE * 3),
               ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_width(ap_ta_ssid, (LV_HOR_RES / 2));
  lv_obj_align_to(label_ssid, ap_ta_ssid, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  std::string tmp_str;
  char out_str[255] = {0};
  const ESP3DSettingDescription *settingPtr =
      esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_ap_ssid);
  if (settingPtr) {
    ssid_ini = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_ap_ssid,
                                           out_str, settingPtr->size);
    lv_textarea_set_text(ap_ta_ssid, ssid_ini.c_str());
    ssid_current = ssid_ini;
  } else {
    esp3d_log_e("This setting is unknown");
  }

  // Password
  lv_obj_t *label_pwd = lv_label_create(ui_new_screen);
  lv_label_set_text(label_pwd, LV_SYMBOL_UNLOCK);
  ESP3DStyle::apply(label_pwd, ESP3DStyleType::bg_label);

  ap_ta_password = lv_textarea_create(ui_new_screen);
  lv_textarea_set_password_mode(ap_ta_password, false);
  lv_textarea_set_password_bullet(ap_ta_password, "•");
  lv_textarea_set_max_length(ap_ta_password, 64);
  lv_textarea_set_one_line(ap_ta_password, true);
  lv_obj_set_width(ap_ta_password, (LV_HOR_RES / 2));

  lv_obj_align_to(ap_ta_password, ap_ta_ssid, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_align_to(
      label_pwd, ap_ta_password, LV_ALIGN_OUT_LEFT_MID,
      -(ESP3D_BUTTON_PRESSED_OUTLINE + lv_obj_get_width(label_pwd) / 2), 0);

  settingPtr =
      esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_ap_password);
  if (settingPtr) {
    password_ini = esp3dTftsettings.readString(
        ESP3DSettingIndex::esp3d_ap_password, out_str, settingPtr->size);
    lv_textarea_set_text(ap_ta_password, password_ini.c_str());
    password_current = password_ini;
  } else {
    esp3d_log_e("This setting is unknown");
  }
  lv_textarea_set_password_mode(ap_ta_password, true);

  wifiStatus::create(ui_new_screen, btnback);

  // Keyboard
  lv_obj_t *kb = lv_keyboard_create(ui_new_screen);
  lv_keyboard_set_textarea(kb, NULL);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_radius(kb, ESP3D_BUTTON_RADIUS , LV_PART_MAIN);
  lv_obj_add_event_cb(ap_ta_ssid, ap_ta_event_cb, LV_EVENT_ALL, kb);
  lv_obj_add_event_cb(ap_ta_password, ap_ta_event_cb, LV_EVENT_ALL, kb);

  // Create button and label for ok
  btn_ok = symbolButton::create(
      ui_new_screen, LV_SYMBOL_OK, ESP3D_SYMBOL_BUTTON_WIDTH, ESP3D_SYMBOL_BUTTON_WIDTH);

  lv_obj_add_event_cb(btn_ok, ap_event_button_ok_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_align(btn_ok, LV_ALIGN_TOP_RIGHT, -ESP3D_BUTTON_PRESSED_OUTLINE,
               ESP3D_BUTTON_PRESSED_OUTLINE);

  // Create button and label for ok
  btn_save = symbolButton::create(
      ui_new_screen, LV_SYMBOL_SAVE, ESP3D_SYMBOL_BUTTON_WIDTH, ESP3D_SYMBOL_BUTTON_WIDTH);
  lv_obj_add_event_cb(btn_save, ap_event_button_save_handler, LV_EVENT_CLICKED,
                      NULL);

  lv_obj_align_to(btn_save, btn_ok, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  update_button_ok();
  update_button_save();
  esp3dTftui.set_current_screen(ESP3DScreenType::access_point);
}
}  // namespace apScreen
#endif  // ESP3D_WIFI_FEATURE