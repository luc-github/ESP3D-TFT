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

#include "auto_leveling_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "leveling_screen.h"

/*
Command
G29 V4
*/
/*
Response:
G29 Auto Bed Leveling
Bed X: 50.000 Y: 50.000 Z: -0.013
Bed X: 133.000 Y: 50.000 Z: 0.009
Bed X: 216.000 Y: 50.000 Z: -0.020
Bed X: 299.000 Y: 50.000 Z: -0.056
Bed X: 299.000 Y: 133.000 Z: -0.010
Bed X: 216.000 Y: 133.000 Z: -0.055
Bed X: 133.000 Y: 133.000 Z: -0.040
Bed X: 50.000 Y: 133.000 Z: -0.049
Bed X: 50.000 Y: 216.000 Z: -0.055
Bed X: 133.000 Y: 216.000 Z: 0.044
Bed X: 216.000 Y: 216.000 Z: 0.044
Bed X: 299.000 Y: 216.000 Z: 0.025
Bed X: 299.000 Y: 299.000 Z: -0.023
Bed X: 216.000 Y: 299.000 Z: -0.058
Bed X: 133.000 Y: 299.000 Z: -0.036
Bed X: 50.000 Y: 299.000 Z: -0.054
Bilinear Leveling Grid:
       0       1       2       3
 0 -0.0125 +0.0087 -0.0200 -0.0563
 1 -0.0488 -0.0400 -0.0550 -0.0100
 2 -0.0550 +0.0437 +0.0437 +0.0250
 3 -0.0538 -0.0363 -0.0575 -0.0225
echo:enqueueing "G0 F3600 X180 Y180"
echo:Settings Stored (526 bytes; crc 45727)
X:50.00 Y:299.00 Z:13.55 E:0.00 Count X:4000 Y:23920 Z:10800
ok
*/

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace autoLevelingScreen {
lv_timer_t *auto_leveling_screen_delay_timer = NULL;

bool auto_leveling_value_cb(ESP3DValuesIndex index, const char *value,
                            ESP3DValuesCbAction action) {
  esp3d_log("auto_leveling_value_cb %d, action: %d, value: %s", (uint16_t)index,
            (uint16_t)action, value);
  return true;
}

void auto_leveling_screen_delay_timer_cb(lv_timer_t *timer) {
  if (auto_leveling_screen_delay_timer) {
    lv_timer_del(auto_leveling_screen_delay_timer);
    auto_leveling_screen_delay_timer = NULL;
  }
  levelingScreen::leveling_screen(true);
}

void event_button_fan_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (auto_leveling_screen_delay_timer) return;
    auto_leveling_screen_delay_timer = lv_timer_create(
        auto_leveling_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    auto_leveling_screen_delay_timer_cb(NULL);
  }
}

void auto_leveling_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Auto leveling screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_fan_back_handler, LV_EVENT_CLICKED,
                      NULL);

  esp3dTftui.set_current_screen(ESP3DScreenType::auto_leveling);
}

}  // namespace autoLevelingScreen