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
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "leveling_screen.h"
#include "rendering/esp3d_rendering_client.h"
#include "translations/esp3d_translation_service.h"

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
lv_timer_t *auto_leveling_screen_probe_delay_timer = NULL;
bool auto_leveling_started = false;
lv_obj_t *auto_leveling_screen_table = NULL;
lv_obj_t *btn_back = NULL;
lv_obj_t *btn_start = NULL;
lv_obj_t *label_status = nullptr;
bool homing_done = false;

double get_value(const char *str, const char *value) {
  char *pstr = strstr(str, value);
  if (pstr == NULL) return 0;
  pstr += strlen(value);
  double val = strtod(pstr, NULL);
  return val;
}

void auto_leveling_screen_probe_delay_timer_cb(lv_timer_t *timer) {
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::auto_leveling) {
    return;
  }
  if (auto_leveling_screen_probe_delay_timer) {
    lv_timer_del(auto_leveling_screen_probe_delay_timer);
    auto_leveling_screen_probe_delay_timer = NULL;
  }
  const char *text = (const char *)timer->user_data;
  esp3d_log("Update status %s", text);
  lv_label_set_text(label_status, text);
}

bool auto_leveling_value_cb(ESP3DValuesIndex index, const char *value,
                            ESP3DValuesCbAction action) {
  static uint8_t col = 0;
  static uint8_t row = 0;
  static double x = 0;
  static double y = 0;
  static uint8_t nb_points = 0;
  if (esp3dTftui.get_current_screen() != ESP3DScreenType::auto_leveling) {
    return false;
  }
  if (index == ESP3DValuesIndex::bed_leveling) {
    if (action == ESP3DValuesCbAction::Add) {
      esp3d_log("Start auto leveling");
      auto_leveling_started = true;
      nb_points = 0;
      std::string text =
          esp3dTranslationService.translate(ESP3DLabel::start_probing);
      lv_label_set_text(label_status, text.c_str());
      return true;
    } else if (action == ESP3DValuesCbAction::Delete) {
      esp3d_log("Stop auto leveling");
      if (auto_leveling_started) {
        lv_label_set_text(label_status, "");
        lv_obj_clear_flag(btn_start, LV_OBJ_FLAG_HIDDEN);
      }
      auto_leveling_started = false;
      return true;
    } else if (action == ESP3DValuesCbAction::Update && auto_leveling_started) {
      esp3d_log("Collect auto leveling data");
      double new_x = get_value(value, "X:");
      double new_y = get_value(value, "Y:");
      double z = get_value(value, "Z:");
      esp3d_log("Got X: %f Y: %f Z: %f", new_x, new_y, z);
      int8_t max_col = -1;
      if (nb_points == 0) {
        col = 0;
        row = 0;
        x = new_x;
        y = new_y;
      } else {
        if (new_x == x) {
          if (max_col == -1 && col > 0) {
            esp3d_log("Max col found is:%d", max_col);
            max_col = col;
          }
          if (new_y > y) {
            row++;
          } else {
            row--;
          }
        } else {
          if (new_x > x) {
            col++;
            if (row == 0) {
              lv_table_set_col_cnt(auto_leveling_screen_table, col + 1);
              for (uint8_t i = 0; i <= col; i++) {
                lv_table_set_col_width(
                    auto_leveling_screen_table, i,
                    ((LV_HOR_RES) - (2 * CURRENT_BUTTON_PRESSED_OUTLINE)) /
                        (col + 1));
              }
            }
          } else {
            col--;
          }
        }
        x = new_x;
        y = new_y;
      }

      std::string str = esp3d_string::set_precision(std::to_string(z), 3);
      esp3d_log_w("Row: %d Col: %d Z: %f , string: %s", row, col, z,
                  str.c_str());
      lv_table_set_cell_value(auto_leveling_screen_table, row, col,
                              str.c_str());
      static std::string text;

      text = esp3dTranslationService.translate(
          ESP3DLabel::auto_bed_probing, std::to_string(nb_points + 1).c_str());
      auto_leveling_screen_probe_delay_timer = lv_timer_create(
          auto_leveling_screen_probe_delay_timer_cb, 1, (void *)text.c_str());

      if (max_col != -1 && col == max_col && row == 1) {
        esp3d_log(
            "Max col found is:%d, col wdith:%d", max_col,
            ((LV_HOR_RES) - (2 * CURRENT_BUTTON_PRESSED_OUTLINE)) / max_col);
        lv_table_set_col_cnt(auto_leveling_screen_table, max_col + 1);

        for (uint8_t i = 0; i <= max_col; i++) {
          lv_table_set_col_width(
              auto_leveling_screen_table, i,
              ((LV_HOR_RES) - (2 * CURRENT_BUTTON_PRESSED_OUTLINE)) /
                  (max_col + 1));
        }
      }
      nb_points++;

      return true;
    }
  }
  esp3d_log("auto_leveling_value_cb %d, action: %d, value: %s", (uint16_t)index,
            (uint16_t)action, value);
  return false;
}

void auto_leveling_screen_delay_timer_cb(lv_timer_t *timer) {
  if (auto_leveling_screen_delay_timer) {
    lv_timer_del(auto_leveling_screen_delay_timer);
    auto_leveling_screen_delay_timer = NULL;
  }
  if (auto_leveling_started) {
    auto_leveling_started = false;
    // TODO send abort command if any and if supported / have emergency parser
    // available
  }
  if (auto_leveling_screen_probe_delay_timer) {
    lv_timer_del(auto_leveling_screen_probe_delay_timer);
    auto_leveling_screen_probe_delay_timer = NULL;
  }
  lv_obj_clear_flag(btn_start, LV_OBJ_FLAG_HIDDEN);
  levelingScreen::leveling_screen(true);
}

void event_button_auto_leveling_start_handler(lv_event_t *e) {
  esp3d_log("start Clicked");
  if (auto_leveling_started) return;
  if (!homing_done) {
    renderingClient.sendGcode("G28");
    homing_done = true;
  }
  esp3d_log("start leveling");
  lv_obj_add_flag(btn_start, LV_OBJ_FLAG_HIDDEN);
  // clear table
  // set one row
  lv_table_set_row_cnt(auto_leveling_screen_table, 0);

  renderingClient.sendGcode("G29 V4");
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
  homing_done = false;
  btn_back = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btn_back, event_button_fan_back_handler, LV_EVENT_CLICKED,
                      NULL);
  lv_obj_update_layout(btn_back);
  // Create a table
  auto_leveling_screen_table = lv_table_create(ui_new_screen);
  lv_obj_set_size(auto_leveling_screen_table,
                  (LV_HOR_RES) - (2 * CURRENT_BUTTON_PRESSED_OUTLINE),
                  LV_VER_RES - (3 * CURRENT_BUTTON_PRESSED_OUTLINE) -
                      lv_obj_get_height(btn_back));
  lv_obj_set_pos(auto_leveling_screen_table, CURRENT_BUTTON_PRESSED_OUTLINE,
                 CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_clear_flag(auto_leveling_screen_table, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_style_radius(auto_leveling_screen_table,
                          CURRENT_BUTTON_RADIUS_VALUE, 0);
  // button start
  btn_start = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_PLAY, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT);
  lv_obj_align_to(btn_start, auto_leveling_screen_table,
                  LV_ALIGN_OUT_BOTTOM_MID, 0, CURRENT_BUTTON_PRESSED_OUTLINE);

  lv_obj_add_event_cb(btn_start, event_button_auto_leveling_start_handler,
                      LV_EVENT_CLICKED, NULL);

  label_status = lv_label_create(ui_new_screen);
  apply_style(label_status, ESP3DStyleType::bg_label);
  lv_label_set_text(label_status, "");
  lv_obj_align_to(label_status, auto_leveling_screen_table,
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_width(label_status, LV_HOR_RES -
                                     (3 * CURRENT_BUTTON_PRESSED_OUTLINE) -
                                     lv_obj_get_width(btn_start));
  lv_label_set_long_mode(label_status, LV_LABEL_LONG_SCROLL_CIRCULAR);
  esp3dTftui.set_current_screen(ESP3DScreenType::auto_leveling);
}

}  // namespace autoLevelingScreen