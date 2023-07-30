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

#include "wifi_status_component.h"

#include <string>

#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace wifiStatus {
/**********************
 *  STATIC VARIABLES
 **********************/
lv_obj_t *wifi_mode_label = nullptr;
lv_obj_t *wifi_status_led = nullptr;
lv_obj_t *wifi_signal = nullptr;

void wifi_display_led() {
  lv_obj_add_flag(wifi_status_led, LV_OBJ_FLAG_HIDDEN);
}
void wifi_display_signal() { lv_obj_add_flag(wifi_signal, LV_OBJ_FLAG_HIDDEN); }
void wifi_display_mode() {
  std::string label_text =
      esp3dTftValues.get_string_value(ESP3DValuesIndex::network_mode);
  lv_label_set_text(wifi_mode_label, label_text.c_str());
}

lv_obj_t *wifi_status(lv_obj_t *parent, lv_obj_t *btnback) {
  lv_obj_update_layout(btnback);
  // Connection status
  wifi_status_led = lv_led_create(parent);
  lv_led_set_brightness(wifi_status_led, 255);
  lv_obj_align_to(wifi_status_led, btnback, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_led_set_color(wifi_status_led, lv_palette_main(LV_PALETTE_RED));
  // signal
  wifi_signal = lv_label_create(parent);
  lv_obj_align_to(wifi_signal, btnback, LV_ALIGN_OUT_LEFT_MID,
                  -CURRENT_BUTTON_PRESSED_OUTLINE, 0);
  lv_label_set_text(wifi_signal, "?");

  // Connection type
  wifi_mode_label = lv_label_create(parent);
  lv_obj_align_to(wifi_mode_label, wifi_status_led, LV_ALIGN_OUT_LEFT_MID, 0,
                  0);
  lv_label_set_text(wifi_mode_label, "?");

  wifi_display_led();
  wifi_display_signal();
  wifi_display_mode();
  return wifi_mode_label;
}

bool network_status_value_cb(ESP3DValuesIndex index, const char *value,
                             ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::wifi ||
        esp3dTftui.get_current_screen() == ESP3DScreenType::station ||
        esp3dTftui.get_current_screen() == ESP3DScreenType::access_point) {
      wifi_display_led();
      wifi_display_signal();
      wifi_display_mode();
    } else {
      // Todo : update other screens calling each callback update function
    }
  }

  return true;
}
bool network_mode_value_cb(ESP3DValuesIndex index, const char *value,
                           ESP3DValuesCbAction action) {
  if (action == ESP3DValuesCbAction::Update) {
    if (esp3dTftui.get_current_screen() == ESP3DScreenType::wifi ||
        esp3dTftui.get_current_screen() == ESP3DScreenType::station ||
        esp3dTftui.get_current_screen() == ESP3DScreenType::access_point) {
      // todo
    } else {
      // Todo : update other screens calling each callback update function
    }
  }
  return true;
}

}  // namespace wifiStatus