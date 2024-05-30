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
#include "wifi_status_component.h"

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "network/esp3d_network.h"
#include "screens/ap_screen.h"
#include "screens/sta_screen.h"
#include "screens/wifi_screen.h"

/**********************
 *  Namespace
 **********************/
namespace wifiStatus {
/**********************
 *  STATIC VARIABLES
 **********************/
lv_obj_t *wifi_mode_label = nullptr;
lv_obj_t *wifi_signal = nullptr;
lv_obj_t *slash_overlay = nullptr;

/**
 * @brief: Displays the WiFi signal strength.
 */
void wifi_display_signal() {
  if (!lv_obj_is_valid(wifi_signal)) {
    esp3d_log_e("wifi_signal object is not valid");
    return;
  }
  std::string mode =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_mode);
  std::string status =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_status);
  if (mode == LV_SYMBOL_STATION_MODE) {
    lv_obj_clear_flag(wifi_signal, LV_OBJ_FLAG_HIDDEN);
    if (status == "+") {
      wifi_ap_record_t ap;
      esp_err_t res = esp_wifi_sta_get_ap_info(&ap);
      if (res == ESP_OK) {
        int32_t signal = esp3dNetwork.getSignal(ap.rssi, false);
        std::string tmpstr = std::to_string(signal);
        tmpstr += "%";
        lv_label_set_text(wifi_signal, tmpstr.c_str());
      } else {
        lv_label_set_text(wifi_signal, "?%");
      }
    } else {
      lv_label_set_text(wifi_signal, "?");
    }
  } else {
    lv_obj_add_flag(wifi_signal, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * @brief: Displays the current WiFi status mode.
 */
void wifi_display_mode() {
  if (!lv_obj_is_valid(wifi_mode_label)) {
    esp3d_log_e("wifi_mode_label object is not valid");
    return;
  }
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_mode);
  lv_label_set_text(wifi_mode_label, label_text.c_str());
  if (label_text == LV_SYMBOL_WIFI) {
    lv_obj_clear_flag(slash_overlay, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(slash_overlay, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * @brief Creates a WiFi status component.
 *
 * This function creates a WiFi status component and adds it to the specified
 * parent object. It also takes a button object as a parameter to be used as a
 * back button.
 *
 * @param parent The parent object to which the WiFi status component will be
 * added.
 * @param btnback The button object to be used as a back button.
 * @return The created WiFi status component object.
 */
lv_obj_t *create(lv_obj_t *parent, lv_obj_t *btnback) {
  lv_obj_update_layout(btnback);
  lv_obj_t *status_container = lv_obj_create(parent);
  if (!lv_obj_is_valid(status_container)) {
    esp3d_log_e("Failed to create status container");
    return nullptr;
  }
  ESP3DStyle::apply(status_container, ESP3DStyleType::default_style);
  lv_obj_clear_flag(status_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(status_container,
                  LV_HOR_RES - (lv_obj_get_width(btnback) +
                                (2 * ESP3D_BUTTON_PRESSED_OUTLINE)),
                  LV_SIZE_CONTENT);
  lv_obj_set_style_pad_all(status_container, 0, LV_PART_MAIN);
  lv_obj_set_style_layout(status_container, LV_LAYOUT_FLEX, LV_PART_MAIN);
  lv_obj_set_style_flex_flow(status_container, LV_FLEX_FLOW_ROW_REVERSE,
                             LV_PART_MAIN);
  lv_obj_set_style_pad_column(status_container,
                              ESP3D_BUTTON_PRESSED_OUTLINE / 2, LV_PART_MAIN);
  lv_obj_set_style_flex_main_place(status_container, LV_FLEX_ALIGN_END,
                                   LV_PART_MAIN);

  // signal
  wifi_signal = lv_label_create(status_container);
  if (!lv_obj_is_valid(wifi_signal)) {
    esp3d_log_e("Failed to create wifi signal label");
    return nullptr;
  }

  lv_label_set_text(wifi_signal, "?");

  // Connection type
  wifi_mode_label = lv_label_create(status_container);
  if (!lv_obj_is_valid(wifi_mode_label)) {
    esp3d_log_e("Failed to create wifi mode label");
    return nullptr;
  }

  lv_label_set_text(wifi_mode_label, "?");
  lv_obj_update_layout(status_container);
  lv_obj_align_to(status_container, btnback, LV_ALIGN_OUT_LEFT_MID,
                  -ESP3D_BUTTON_PRESSED_OUTLINE, 0);
  slash_overlay = lv_label_create(wifi_mode_label);
  lv_label_set_text(slash_overlay, LV_SYMBOL_SLASH);
  lv_obj_center(slash_overlay);

  wifi_display_signal();
  wifi_display_mode();
  return status_container;
}

/**
 * @brief Callback function for handling network status values.
 *
 * This function is called when a network status value is received from the
 * ESP3D server. It takes the index of the value, the value itself, and the
 * action to be performed.
 *
 * @param index The index of the network status value.
 * @param value The value of the network status.
 * @param action The action to be performed on the network status value.
 * @return bool Returns true if the callback was successful, false otherwise.
 */
bool network_status_cb(ESP3DValuesIndex index, const char *value,
                       ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::wifi ||
        esp3dTftui.get_current_screen() == ESP3DScreenType::station ||
        esp3dTftui.get_current_screen() == ESP3DScreenType::access_point) {
      wifi_display_signal();
      wifi_display_mode();

    } else {
      // Todo : update other screens calling each callback update function
    }
  }

  return true;
}

/**
 * Callback function for handling network mode values.
 *
 * This function is called when a network mode value is received from ESP3D.
 *
 * @param index The index of the network mode value.
 * @param value The value of the network mode.
 * @param action The action to be performed on the network mode value.
 * @return `true` if the network mode value was handled successfully, `false`
 * otherwise.
 */
bool network_mode_cb(ESP3DValuesIndex index, const char *value,
                     ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::wifi ||
        esp3dTftui.get_current_screen() == ESP3DScreenType::station ||
        esp3dTftui.get_current_screen() == ESP3DScreenType::access_point) {
      wifi_display_signal();
      wifi_display_mode();
      if (esp3dTftui.get_current_screen() == ESP3DScreenType::access_point) {
        apScreen::callback();
      }
      if (esp3dTftui.get_current_screen() == ESP3DScreenType::station) {
        staScreen::callback();
        staScreen::callback_scan();
      }
      if (esp3dTftui.get_current_screen() == ESP3DScreenType::wifi) {
        wifiScreen::callback();
      }
    } else {
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}

}  // namespace wifiStatus
#endif  // ESP3D_WIFI_FEATURE