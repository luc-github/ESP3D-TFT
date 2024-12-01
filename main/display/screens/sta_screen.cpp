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
#include "screens/sta_screen.h"

#include <lvgl.h>

#include <list>

#include "bsp.h"
#include "components/back_button_component.h"
#include "components/list_line_component.h"
#include "components/main_container_component.h"
#include "components/message_box_component.h"
#include "components/spinner_component.h"
#include "components/symbol_button_component.h"
#include "components/wifi_status_component.h"
#include "esp3d_log.h"
#include "esp3d_hal.h"
#include "esp3d_lvgl.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "network/esp3d_network.h"
#include "screens/wifi_screen.h"
#include "tasks_def.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  Namespace
 **********************/
namespace staScreen {
#define STACKDEPTH 4096
#define TASKPRIORITY UI_TASK_PRIORITY - 1
#define TASKCORE UI_TASK_CORE

#define MAX_SCAN_LIST_SIZE 10

// Static variables
lv_timer_t *sta_screen_delay_timer = NULL;
lv_obj_t *sta_ta_ssid = NULL;
lv_obj_t *sta_ta_password = NULL;
lv_obj_t *btn_ok = nullptr;
lv_obj_t *btn_scan = nullptr;
lv_obj_t *btn_save = nullptr;
lv_obj_t *ui_sta_ssid_list_ctl = NULL;
lv_timer_t *start_scan_timer = NULL;
lv_obj_t *status_component = nullptr;

std::string ssid_ini;
std::string password_ini;
std::string ssid_current;
std::string password_current;
void update_button_save();

struct ESP3DSSIDDescriptor {
  std::string ssid;
  std::string isprotected;
  std::string signal_strength;
};
std::list<ESP3DSSIDDescriptor> ssid_scanned_list;

// Static functions prototypes
void fill_ssid_scanned_list();
void fill_ui_sta_ssid_list();

#if ESP3D_PATCH_DELAY_REFRESH
lv_timer_t *sta_screen_delay_refresh_timer = NULL;
/**
 * @brief Callback function for the STA screen delay refresh timer.
 *
 * This function is called when the delay refresh timer expires. It is
 * responsible for hiding the spinner screen and cleaning up the timer.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void sta_screen_delay_refresh_timer_cb(lv_timer_t *timer) {
  esp3d_log("Refresh timer_cb");
  if (sta_screen_delay_refresh_timer &&
      lv_timer_is_valid(sta_screen_delay_refresh_timer)) {
    lv_timer_del(sta_screen_delay_refresh_timer);
  }
  sta_screen_delay_refresh_timer = NULL;
  spinnerScreen::hide();
}
#endif  // ESP3D_PATCH_DELAY_REFRESH

/**
 * @brief Callback function to refresh the scanned SSID list.
 *
 * This function is called when a timer expires. It is responsible for
 * refreshing the scanned SSID list by calling the `fill_ui_sta_ssid_list`
 * function. It also cancels the timer if it is running.
 *
 * @param timer A pointer to the timer that triggered the callback.
 */
void refresh_ssid_scanned_list_cb(lv_timer_t *timer) {
  esp3d_log("Refresh list");
  if (start_scan_timer && lv_timer_is_valid(start_scan_timer)) {
    lv_timer_del(start_scan_timer);
  }
  start_scan_timer = NULL;
  fill_ui_sta_ssid_list();
}
/**
 * @brief Background scan task function.
 *
 * This function is responsible for performing a background scan of available
 * Wi-Fi networks. It fills the scanned SSID list and creates a timer for
 * refreshing the list periodically.
 *
 * @param pvParameter Pointer to the task parameter (not used).
 */
static void bgScanTask(void *pvParameter) {
  esp3d_log("Scan task");
  (void)pvParameter;
  esp3d_hal::wait(100);
  fill_ssid_scanned_list();
  if (!start_scan_timer) {
    start_scan_timer = lv_timer_create(refresh_ssid_scanned_list_cb, 100, NULL);
  }
  esp3d_log("Scan task end");
  vTaskDelete(NULL);
}

/**
 * @brief Fills the `ssid_scanned_list` with scanned SSID information.
 *
 * This function clears the `ssid_scanned_list` and performs a WiFi scan to
 * retrieve information about nearby access points (APs). It populates the
 * `ssid_scanned_list` with the SSID descriptors of the scanned APs, including
 * the SSID name, signal strength, and protection status.
 *
 * @note This function requires WiFi to be enabled. If WiFi is turned off, the
 * function will log an error and return.
 *
 * @note If the current WiFi mode is not set to station mode (wifi_sta), the
 * function will temporarily switch the WiFi mode to AP+STA mode
 * (WIFI_MODE_APSTA) to perform the scan.
 *
 * @note The maximum number of scanned APs is defined by MAX_SCAN_LIST_SIZE.
 *
 * @note The `ssid_scanned_list` is cleared before populating it with the
 * scanned SSID descriptors.
 *
 * @return None.
 */
void fill_ssid_scanned_list() {
  ssid_scanned_list.clear();
  if (esp3dNetwork.getMode() == ESP3DRadioMode::off) {
    esp3d_log_e("Wifi is off so no scan possible");
    return;
  }
  if (esp3dNetwork.getMode() != ESP3DRadioMode::wifi_sta) {
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  }
  esp_err_t res = esp_wifi_scan_start(NULL, true);

  if (res == ESP_OK) {
    uint16_t number = MAX_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[MAX_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    esp3d_log("Got %d ssid in scan", ap_count);
    for (int i = 0; (i < MAX_SCAN_LIST_SIZE) && (i < ap_count); i++) {
      esp3d_log("%s %d %d", ap_info[i].ssid, ap_info[i].rssi,
                ap_info[i].authmode);

      int32_t signal = esp3dNetwork.getSignal(ap_info[i].rssi, false);
      if (signal != 0) {
        ESP3DSSIDDescriptor ssid_desc;
        ssid_desc.ssid = (const char *)ap_info[i].ssid;
        if (signal < 10)
          ssid_desc.signal_strength = "  ";
        else if (signal < 100)
          ssid_desc.signal_strength = " ";
        else
          ssid_desc.signal_strength = "";
        ssid_desc.signal_strength += std::to_string(signal) + std::string("%");
        if (ap_info[i].authmode != WIFI_AUTH_OPEN) {
          ssid_desc.isprotected = LV_SYMBOL_LOCK;
        } else {
          ssid_desc.isprotected = " ";
        }
        ssid_scanned_list.push_back(ssid_desc);
      }
    }
    esp_wifi_clear_ap_list();
  }
  if (esp3dNetwork.getMode() != ESP3DRadioMode::wifi_sta) {
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  }
}

/**
 * Callback function for the "Join" button event.
 * Sets the selected SSID as the text in the SSID textarea.
 *
 * @param e The event object.
 */
void event_button_join_cb(lv_event_t *e) {
  ESP3DSSIDDescriptor *ssid_desc =
      (ESP3DSSIDDescriptor *)lv_event_get_user_data(e);
  esp3d_log("You choosed %s", ssid_desc->ssid.c_str());
  lv_textarea_set_text(sta_ta_ssid, ssid_desc->ssid.c_str());
}

/**
 * Callback function for the background touch event.
 * Hides the ssid list control and clears the hidden flag of the status
 * component.
 *
 * @param e Pointer to the event object.
 */
void event_bg_touched_cb(lv_event_t *e) {
  if (ui_sta_ssid_list_ctl)
    lv_obj_add_flag(ui_sta_ssid_list_ctl, LV_OBJ_FLAG_HIDDEN);
  if (status_component) lv_obj_clear_flag(status_component, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief Fills the UI with the list of available SSIDs.
 *
 * This function iterates over the `ssid_scanned_list` and creates a line
 * container for each SSID. It adds labels for the SSID, signal strength, and
 * protection status to the line container. Additionally, a button with the
 * LV_SYMBOL_OK symbol is added to each line container. The button is assigned
 * an event callback function `event_button_join_cb` for the LV_EVENT_CLICKED
 * event.
 *
 * @note This function assumes that the `ssid_scanned_list` is populated with
 * the available SSIDs.
 *
 * @see listLine::create
 * @see listLine::add_label
 * @see listLine::add_button
 * @see event_button_join_cb
 * @see spinnerScreen::hide
 */
void fill_ui_sta_ssid_list() {
  for (auto &ssid_desc : ssid_scanned_list) {
    lv_obj_t *line_container = listLine::create(ui_sta_ssid_list_ctl);
    if (!lv_obj_is_valid(line_container)) {
      esp3d_log_e("Failed to create line container");
      return;
    }
    listLine::add_label(ssid_desc.ssid.c_str(), line_container, true);

    listLine::add_label(ssid_desc.signal_strength.c_str(), line_container,
                        false);
    listLine::add_label(ssid_desc.isprotected.c_str(), line_container, false);
    lv_obj_t *btnJoin = listLine::add_button(LV_SYMBOL_OK, line_container);
    lv_obj_add_event_cb(btnJoin, event_button_join_cb, LV_EVENT_CLICKED,
                        (void *)&ssid_desc);
  }
  spinnerScreen::hide();
}

/**
 * @brief Initiates a scan task.
 *
 * This function creates a new task called "scanTask" and assigns it to a
 * specific core. The task is responsible for performing a background scan.
 *
 * @note This function logs the free heap size before and after creating the
 * task.
 *
 * @return None.
 */
void do_scan_now() {
  TaskHandle_t xHandle = NULL;
  BaseType_t res =
      xTaskCreatePinnedToCore(bgScanTask, "scanTask", STACKDEPTH, NULL,
                              TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Scan Task freeheap %u, %u",
              (unsigned int)esp_get_free_heap_size(),
              (unsigned int)heap_caps_get_free_size(MALLOC_CAP_8BIT |
                                                    MALLOC_CAP_INTERNAL));
  } else {
    esp3d_log_e("Scan Task creation failed %d , %d, freeheap %u, %u", (int)res,
                (int)xHandle, (unsigned int)esp_get_free_heap_size(),
                (unsigned int)heap_caps_get_free_size(MALLOC_CAP_8BIT |
                                                      MALLOC_CAP_INTERNAL));
  }
}

/**
 * @brief Callback function for the start scan timer.
 *
 * This function is called when the start scan timer expires. It is responsible
 * for stopping the timer and calling the `do_scan_now` function to initiate the
 * scanning process.
 *
 * @param timer A pointer to the timer object that triggered the callback.
 */
void start_scan_timer_cb(lv_timer_t *timer) {
  if (start_scan_timer) {
    lv_timer_del(start_scan_timer);
    start_scan_timer = NULL;
  }
  do_scan_now();
}

/**
 * Saves the parameters for the ESP3D TFT screen.
 * This function saves the current SSID and password values to the ESP3D
 * settings. If the write operation fails, an error message is displayed and the
 * function returns false. Otherwise, the initial values are updated and the
 * save button is updated.
 *
 * @return true if the parameters are successfully saved, false otherwise.
 */
bool save_parameters() {
  esp3d_log("Save parameters");
  bool res = true;
  if (!esp3dTftsettings.writeString(ESP3DSettingIndex::esp3d_sta_ssid,
                                    ssid_current.c_str())) {
    std::string text =
        esp3dTranslationService.translate(ESP3DLabel::error_applying_mode);
    msgBox::create(NULL, MsgBoxType::error, text.c_str());
    esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                    text.c_str());
    res = false;
  } else {
    ssid_ini = ssid_current;
    if (!esp3dTftsettings.writeString(ESP3DSettingIndex::esp3d_sta_password,
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

/**
 * @brief Callback function for scanning network mode.
 *
 * This function is called when the current screen is the station screen.
 * It checks the network mode and hides or shows the scan button accordingly.
 */
void callback_scan() {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::station) return;
  std::string mode =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_mode);
  if (mode == LV_SYMBOL_WIFI) {
    lv_obj_add_flag(btn_scan, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(btn_scan, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * @brief This function is a callback function used in the STA screen.
 *        It updates the visibility of certain elements based on the network
 * mode and status. If the network mode is not STA or if the required settings
 * are not valid, certain elements are hidden. Otherwise, the elements are made
 * visible. Additionally, if the network mode is AP or NO WIFI and the status is
 * "+", the spinner screen is hidden.
 */
void callback() {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::station) return;
  esp3d_log("Update ok vibility");
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
  if (!esp3dTftsettings.isValidStringSetting(
          ssid_current.c_str(), ESP3DSettingIndex::esp3d_sta_ssid) ||
      (mode == LV_SYMBOL_STATION_MODE && ssid_ini == ssid_current) ||
      !esp3dTftsettings.isValidStringSetting(
          password_current.c_str(), ESP3DSettingIndex::esp3d_sta_password)) {
    esp3d_log("Created Scan Task freeheap %u, %u",
              (unsigned int)esp_get_free_heap_size(),
              (unsigned int)heap_caps_get_free_size(MALLOC_CAP_8BIT |
                                                    MALLOC_CAP_INTERNAL));
    esp3d_log("Ok hide");
    lv_obj_add_flag(btn_ok, LV_OBJ_FLAG_HIDDEN);
#if ESP3D_PATCH_DELAY_REFRESH
    if (sta_screen_delay_refresh_timer &&
        lv_timer_is_valid(sta_screen_delay_refresh_timer)) {
      lv_timer_del(sta_screen_delay_refresh_timer);
    }
    sta_screen_delay_refresh_timer =
        lv_timer_create(sta_screen_delay_refresh_timer_cb, 100, NULL);
#else
    spinnerScreen::hide();
#endif  // ESP3D_PATCH_DELAY_REFRESH
  } else {
    esp3d_log("Ok visible");
    lv_obj_clear_flag(btn_ok, LV_OBJ_FLAG_HIDDEN);
    if ((mode == LV_SYMBOL_ACCESS_POINT && status == "+") ||
        (mode == LV_SYMBOL_WIFI && status == ".")) {
      spinnerScreen::hide();
    }
  }
}

/**
 * @brief Updates the visibility of the save button based on the current SSID
 * and password settings.
 *
 * This function checks if the current SSID and password match the initial
 * values. If they do, the save button is hidden. If the SSID or password has
 * been modified, the save button is visible. Additionally, if the current SSID
 * or password is not a valid string setting, the save button is hidden.
 */
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
          ssid_current.c_str(), ESP3DSettingIndex::esp3d_sta_ssid)) {
    lv_obj_add_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
  }
  if (!esp3dTftsettings.isValidStringSetting(
          password_current.c_str(), ESP3DSettingIndex::esp3d_sta_password)) {
    lv_obj_add_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * Callback function for the STA screen delay timer.
 * This function is called when the delay timer expires.
 * It checks if the delay timer is active, deletes the timer, and creates the
 * WiFi screen if the current screen is not "none" or "wifi".
 *
 * @param timer A pointer to the timer object that triggered the callback.
 */
void sta_screen_delay_timer_cb(lv_timer_t *timer) {
  if (sta_screen_delay_timer && lv_timer_is_valid(sta_screen_delay_timer)) {
    lv_timer_del(sta_screen_delay_timer);
    sta_screen_delay_timer = NULL;
    if (esp3dTftui.get_current_screen() != ESP3DScreenType::none &&
        esp3dTftui.get_current_screen() != ESP3DScreenType::wifi) {
      wifiScreen::create();
    }
  }
}

/**
 * @brief Event handler for the "back" button in the STA screen.
 *
 * This function is called when the "back" button is clicked in the STA screen.
 * It logs a message indicating that the button has been clicked and checks if a
 * delay timer is already running. If a delay timer is running, it deletes the
 * timer and sets it to NULL. Then, it creates a new delay timer with the
 * specified callback function and delay duration.
 *
 * @param e Pointer to the event object.
 */
void event_button_sta_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (sta_screen_delay_timer && lv_timer_is_valid(sta_screen_delay_timer)) {
    esp3d_log_w("Timer is already running, delete it");
    lv_timer_del(sta_screen_delay_timer);
  }

  sta_screen_delay_timer = lv_timer_create(sta_screen_delay_timer_cb,
                                           ESP3D_BUTTON_ANIMATION_DELAY, NULL);
}

/**
 * Callback function for the text area events in the STA screen.
 * Handles various events such as focus, click, defocus, cancel, ready, and
 * value changed. Updates the UI components and performs necessary actions based
 * on the event.
 *
 * @param e The event object containing information about the event.
 */
void sta_ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
    lv_obj_add_flag(btn_scan, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_state(ta, LV_STATE_FOCUSED);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_AUTO);
    if (ta == sta_ta_password) {
      lv_textarea_set_password_mode(ta, false);
    }
    if (ui_sta_ssid_list_ctl)
      lv_obj_add_flag(ui_sta_ssid_list_ctl, LV_OBJ_FLAG_HIDDEN);
    if (status_component)
      lv_obj_clear_flag(status_component, LV_OBJ_FLAG_HIDDEN);

    spinnerScreen::hide();
    callback();
    update_button_save();
  }

  else if (code == LV_EVENT_DEFOCUSED || code == LV_EVENT_CANCEL) {
    lv_obj_clear_flag(btn_scan, LV_OBJ_FLAG_HIDDEN);
    lv_textarea_set_cursor_pos(ta, 0);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    if (ta == sta_ta_password) {
      lv_textarea_set_password_mode(ta, true);
    }
    callback();
    update_button_save();
  } else if (code == LV_EVENT_READY) {
    lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    if (ta == sta_ta_ssid) {
      ssid_current = lv_textarea_get_text(ta);
      esp3d_log("Ready, SSID: %s", lv_textarea_get_text(ta));
    } else if (ta == sta_ta_password) {
      esp3d_log("Ready, PASSWORD: %s", lv_textarea_get_text(ta));
      password_current = lv_textarea_get_text(ta);
    }
    callback();
    update_button_save();
  } else if (code == LV_EVENT_VALUE_CHANGED) {
    if (ta == sta_ta_ssid) {
      ssid_current = lv_textarea_get_text(ta);
      esp3d_log("Value changed, SSID: %s > %s", ssid_ini.c_str(),
                ssid_current.c_str());
    } else if (ta == sta_ta_password) {
      password_current = lv_textarea_get_text(ta);
      esp3d_log("Value changed, PASSWORD: %s > %s", password_ini.c_str(),
                password_current.c_str());
    }
    callback();
    update_button_save();
  }
}

/**
 * Event handler for the scan button in the STA screen.
 * This function is called when the scan button is clicked.
 * It clears the hidden flag of the SSID list control, hides the status
 * component, shows the spinner screen, creates a timer for scanning, and clears
 * the SSID list control.
 *
 * @param e The event object.
 */
void sta_event_button_scan_handler(lv_event_t *e) {
  esp3d_log("scan Clicked");
  if (ui_sta_ssid_list_ctl) {
    lv_obj_clear_flag(ui_sta_ssid_list_ctl, LV_OBJ_FLAG_HIDDEN);
    if (status_component) lv_obj_add_flag(status_component, LV_OBJ_FLAG_HIDDEN);
    spinnerScreen::show();
    start_scan_timer = lv_timer_create(start_scan_timer_cb, 100, NULL);
    size_t i = lv_obj_get_child_cnt(ui_sta_ssid_list_ctl);
    while (i > 0) {
      lv_obj_del(lv_obj_get_child(ui_sta_ssid_list_ctl, 0));
      i--;
    }
  }
}
/**
 * Event handler for the "Ok" button in the STA screen.
 * This function is called when the "Ok" button is clicked.
 * It saves the current SSID and password, applies the changes, and updates the
 * UI accordingly. If the parameters fail to save, the function returns without
 * further action. If the radio mode fails to apply, an error message is
 * displayed and the status bar label is updated. Otherwise, a spinner screen is
 * shown and the network mode is set to WiFi STA asynchronously.
 *
 * @param e The event object associated with the button click.
 */
void sta_event_button_ok_handler(lv_event_t *e) {
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
          static_cast<uint8_t>(ESP3DRadioMode::wifi_sta))) {
    std::string text =
        esp3dTranslationService.translate(ESP3DLabel::error_applying_mode);
    msgBox::create(NULL, MsgBoxType::error, text.c_str());
    esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                    text.c_str());
  } else {
    spinnerScreen::show();
    esp3dNetwork.setModeAsync(ESP3DRadioMode::wifi_sta);
  }
}

/**
 * Event handler for the "Save" button in the STA screen.
 * This function logs the click event and saves the current SSID and password.
 */
void sta_event_button_save_handler(lv_event_t *e) {
  esp3d_log("Save Clicked");
  esp3d_log("ssid: %s, password:%s", ssid_current.c_str(),
            password_current.c_str());
  save_parameters();
}

/**
 * @brief Creates the STA screen.
 *
 * This function creates the STA screen by setting the current screen to none,
 * creating a new screen, loading the new screen, applying the main background
 * style, deleting the old screen, creating and configuring UI elements such as
 * buttons, labels, text areas, and keyboard, and setting up event handlers for
 * button clicks. It also retrieves and sets the initial values for the SSID and
 * password text areas from the settings. Finally, it creates and aligns buttons
 * for OK, Save, and Scan, and initializes the SSID list control.
 */
void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("STA screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_new_screen)) {
    esp3d_log_e("Failed to create screen");
    return;
  }
  // Display new screen and delete old one
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
  lv_obj_add_event_cb(btnback, event_button_sta_back_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_add_event_cb(ui_new_screen, event_bg_touched_cb, LV_EVENT_CLICKED,
                      NULL);
  // SSID
  lv_obj_t *label_ssid = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_ssid)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(label_ssid, LV_SYMBOL_ACCESS_POINT);
  ESP3DStyle::apply(label_ssid, ESP3DStyleType::bg_label);
  lv_obj_update_layout(label_ssid);
  int width_label = lv_obj_get_width(label_ssid);
  esp3d_log("width_label %d", lv_obj_get_width(label_ssid));
  sta_ta_ssid = lv_textarea_create(ui_new_screen);
  if (!lv_obj_is_valid(sta_ta_ssid)) {
    esp3d_log_e("Failed to create text area");
    return;
  }
  lv_textarea_set_one_line(sta_ta_ssid, true);
  lv_textarea_set_max_length(sta_ta_ssid, 32);
  lv_obj_align(sta_ta_ssid, LV_ALIGN_TOP_LEFT,
               width_label + (ESP3D_BUTTON_PRESSED_OUTLINE * 3),
               ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_width(sta_ta_ssid, (LV_HOR_RES / 2));
  lv_obj_align_to(label_ssid, sta_ta_ssid, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  std::string tmp_str;
  char out_str[255] = {0};
  const ESP3DSettingDescription *settingPtr =
      esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_sta_ssid);
  if (settingPtr) {
    ssid_ini = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_sta_ssid,
                                           out_str, settingPtr->size);
    lv_textarea_set_text(sta_ta_ssid, ssid_ini.c_str());
    ssid_current = ssid_ini;
  } else {
    esp3d_log_e("This setting is unknown");
  }

  // Password
  lv_obj_t *label_pwd = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(label_pwd)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(label_pwd, LV_SYMBOL_UNLOCK);
  ESP3DStyle::apply(label_pwd, ESP3DStyleType::bg_label);

  sta_ta_password = lv_textarea_create(ui_new_screen);
  if (!lv_obj_is_valid(sta_ta_password)) {
    esp3d_log_e("Failed to create text area");
    return;
  }
  lv_textarea_set_password_mode(sta_ta_password, false);
  lv_textarea_set_password_bullet(sta_ta_password, "•");
  lv_textarea_set_max_length(sta_ta_password, 64);
  lv_textarea_set_one_line(sta_ta_password, true);
  lv_obj_set_width(sta_ta_password, (LV_HOR_RES / 2));

  lv_obj_align_to(sta_ta_password, sta_ta_ssid, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_align_to(
      label_pwd, sta_ta_password, LV_ALIGN_OUT_LEFT_MID,
      -(ESP3D_BUTTON_PRESSED_OUTLINE + lv_obj_get_width(label_pwd) / 2), 0);

  settingPtr =
      esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_sta_password);
  if (settingPtr) {
    password_ini = esp3dTftsettings.readString(
        ESP3DSettingIndex::esp3d_sta_password, out_str, settingPtr->size);
    lv_textarea_set_text(sta_ta_password, password_ini.c_str());
    password_current = password_ini;
  } else {
    esp3d_log_e("This setting is unknown");
  }
  lv_textarea_set_password_mode(sta_ta_password, true);

  status_component = wifiStatus::create(ui_new_screen, btnback);
  if (!lv_obj_is_valid(status_component)) {
    esp3d_log_e("Failed to create status component");
    return;
  }

  // Keyboard
  lv_obj_t *kb = lv_keyboard_create(ui_new_screen);
  if (!lv_obj_is_valid(kb)) {
    esp3d_log_e("Failed to create keyboard");
    return;
  }
  lv_keyboard_set_textarea(kb, NULL);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_radius(kb, ESP3D_BUTTON_RADIUS, LV_PART_MAIN);
  lv_obj_add_event_cb(sta_ta_ssid, sta_ta_event_cb, LV_EVENT_ALL, kb);
  lv_obj_add_event_cb(sta_ta_password, sta_ta_event_cb, LV_EVENT_ALL, kb);

  // Create button and label for ok
  btn_ok = symbolButton::create(ui_new_screen, LV_SYMBOL_OK,
                                ESP3D_SYMBOL_BUTTON_WIDTH,
                                ESP3D_SYMBOL_BUTTON_WIDTH);
  if (!lv_obj_is_valid(btn_ok)) {
    esp3d_log_e("Failed to create button");
    return;
  }

  lv_obj_add_event_cb(btn_ok, sta_event_button_ok_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_align(btn_ok, LV_ALIGN_TOP_RIGHT, -ESP3D_BUTTON_PRESSED_OUTLINE,
               ESP3D_BUTTON_PRESSED_OUTLINE);

  // Create button and label for ok
  btn_save = symbolButton::create(ui_new_screen, LV_SYMBOL_SAVE,
                                  ESP3D_SYMBOL_BUTTON_WIDTH,
                                  ESP3D_SYMBOL_BUTTON_WIDTH);
  if (!lv_obj_is_valid(btn_save)) {
    esp3d_log_e("Failed to create button");
    return;
  }
  lv_obj_add_event_cb(btn_save, sta_event_button_save_handler, LV_EVENT_CLICKED,
                      NULL);

  lv_obj_align_to(btn_save, btn_ok, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);

  btn_scan = symbolButton::create(ui_new_screen, LV_SYMBOL_SEARCH,
                                  ESP3D_SYMBOL_BUTTON_WIDTH,
                                  ESP3D_SYMBOL_BUTTON_WIDTH);
  if (!lv_obj_is_valid(btn_scan)) {
    esp3d_log_e("Failed to create button");
    return;
  }

  lv_obj_add_event_cb(btn_scan, sta_event_button_scan_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_align_to(btn_scan, btn_ok, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_update_layout(btn_scan);
  // SSID list
  ui_sta_ssid_list_ctl = lv_list_create(ui_new_screen);
  if (!lv_obj_is_valid(ui_sta_ssid_list_ctl)) {
    esp3d_log_e("Failed to create list");
    return;
  }
  lv_obj_clear_flag(ui_sta_ssid_list_ctl, LV_OBJ_FLAG_SCROLL_ELASTIC);
  // lv_obj_update_layout(sta_ta_ssid);
  lv_obj_update_layout(label_ssid);
  lv_obj_align_to(ui_sta_ssid_list_ctl, label_ssid, LV_ALIGN_OUT_BOTTOM_LEFT,
                  -(ESP3D_BUTTON_PRESSED_OUTLINE),
                  (ESP3D_BUTTON_PRESSED_OUTLINE));

  lv_obj_set_width(ui_sta_ssid_list_ctl,
                   LV_HOR_RES - (lv_obj_get_width(btn_scan) +
                                 (3 * ESP3D_BUTTON_PRESSED_OUTLINE)));
  lv_obj_update_layout(ui_sta_ssid_list_ctl);

  lv_obj_set_height(
      ui_sta_ssid_list_ctl,
      (lv_obj_get_y(btnback)) -
          ((lv_obj_get_y(label_ssid) + lv_obj_get_height(label_ssid))));
  lv_obj_update_layout(ui_sta_ssid_list_ctl);

  if (ui_sta_ssid_list_ctl)
    lv_obj_add_flag(ui_sta_ssid_list_ctl, LV_OBJ_FLAG_HIDDEN);
  if (status_component) lv_obj_clear_flag(status_component, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_pad_left(ui_sta_ssid_list_ctl, ESP3D_LIST_CONTAINER_LR_PAD,
                            LV_PART_MAIN);
  lv_obj_set_style_pad_right(ui_sta_ssid_list_ctl, ESP3D_LIST_CONTAINER_LR_PAD,
                             LV_PART_MAIN);

  callback();
  update_button_save();
  callback_scan();
  esp3dTftui.set_current_screen(ESP3DScreenType::station);
}
}  // namespace staScreen
#endif  // ESP3D_WIFI_FEATURE