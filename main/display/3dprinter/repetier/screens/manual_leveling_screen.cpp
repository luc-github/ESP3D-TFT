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

#include "screens/manual_leveling_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/message_box_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "screens/leveling_screen.h"
#include "screens/menu_screen.h"
#include "rendering/esp3d_rendering_client.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace manualLevelingScreen {
bool auto_leveling = false;
bool homing_done = false;
const char *leveling_position_buttons_map[] = {"4",
                                               LV_SYMBOL_BULLET,
                                               "5",
                                               "\n",
                                               LV_SYMBOL_BULLET,
                                               "1",
                                               LV_SYMBOL_BULLET,
                                               "\n",
                                               "2",
                                               LV_SYMBOL_BULLET,
                                               "3",
                                               ""};
int8_t leveling_position_buttons_map_id = -1;
lv_obj_t *btnm_leveling_position = NULL;
lv_obj_t *btn_next = NULL;
lv_obj_t *btn_previous = NULL;
lv_obj_t *btn_start = NULL;
lv_obj_t *status_container = NULL;
double intialization_done = false;
double bed_width = 100;
double bed_depth = 100;
bool invert_x = false;
bool invert_y = false;
#define DEFAULT_Z_DISTANCE 15
lv_timer_t *manual_leveling_screen_delay_timer = NULL;
void bed_width(double value) { bed_width = value; }

void bed_depth(double value) { bed_depth = value; }
void invert_x(bool value) { invert_x = value; }
void invert_y(bool value) { invert_y = value; }

void manual_leveling_screen_delay_timer_cb(lv_timer_t *timer) {
  if (manual_leveling_screen_delay_timer) {
    lv_timer_del(manual_leveling_screen_delay_timer);
    manual_leveling_screen_delay_timer = NULL;
  }
  if (auto_leveling) {
    levelingScreen::create(auto_leveling);
  } else {
    menuScreen::create();
  }
}

void event_button_manual_leveling_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (manual_leveling_screen_delay_timer) return;
    manual_leveling_screen_delay_timer = lv_timer_create(
        manual_leveling_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    manual_leveling_screen_delay_timer_cb(NULL);
  }
}

void move_to_position(int pos) {
  if (!homing_done) {
    esp3d_log("Homing not done, doing now...");
    renderingClient.sendGcode("G28");
    // In theory when homing, if 2 extruders, E0 is set by default
    // TBC if need to explicilty set it to E0 using T0 command
    homing_done = true;
  }
  std::string gcode_command = "G1 Z" + std::to_string(DEFAULT_Z_DISTANCE);
  renderingClient.sendGcode(gcode_command.c_str());
  double x = 0;
  double y = 0;
  double x_pad = bed_width / 10;
  double y_pad = bed_depth / 10;
  switch (pos) {
    case 0:
      // Move to position 0
      esp3d_log("Move to position 0");
      if (invert_x) {
        x = bed_width - x_pad;
      } else {
        x = x_pad;
      }
      if (invert_y) {
        y = y_pad;
      } else {
        y = bed_depth - y_pad;
      }
      break;
    case 1:
      // Move to position 1
      esp3d_log("Move to position 1");
      x = bed_width / 2;
      if (invert_y) {
        y = y_pad;
      } else {
        y = bed_depth - y_pad;
      }
      break;
    case 2:
      // Move to position 2
      esp3d_log("Move to position 2");
      if (invert_x) {
        x = x_pad;
      } else {
        x = bed_width - x_pad;
      }
      if (invert_y) {
        y = y_pad;
      } else {
        y = bed_depth - y_pad;
      }
      break;
    case 3:
      // Move to position 3
      esp3d_log("Move to position 3");
      if (invert_x) {
        x = bed_width - x_pad;
      } else {
        x = x_pad;
      }
      y = bed_depth / 2;
      break;
    case 4:
      // Move to position 4
      esp3d_log("Move to position 4");
      x = bed_width / 2;
      y = bed_depth / 2;
      break;
    case 5:
      // Move to position 5
      esp3d_log("Move to position 5");
      if (invert_x) {
        x = x_pad;
      } else {
        x = bed_width - x_pad;
      }
      y = bed_depth / 2;
      break;
    case 6:
      // Move to position 6
      esp3d_log("Move to position 6");
      if (invert_x) {
        x = bed_width - x_pad;
      } else {
        x = x_pad;
      }
      if (invert_y) {
        y = bed_depth - y_pad;
      } else {
        y = y_pad;
      }
      break;
    case 7:
      // Move to position 7
      esp3d_log("Move to position 7");
      x = bed_width / 2;
      if (invert_y) {
        y = bed_depth - y_pad;
      } else {
        y = y_pad;
      }
      break;
    case 8:
      // Move to position 8
      esp3d_log("Move to position 8");
      if (invert_x) {
        x = x_pad;
      } else {
        x = bed_width - x_pad;
      }
      if (invert_y) {
        y = bed_depth - y_pad;
      } else {
        y = y_pad;
      }
      break;
    default:
      break;
  }
  // Move to position
  gcode_command = "G1 X" + std::to_string(x) + " Y" + std::to_string(y);
  renderingClient.sendGcode(gcode_command.c_str());
  // Move to Z0
  renderingClient.sendGcode("G1 Z0");
}

void event_button_manual_leveling_next_handler(lv_event_t *e) {
  lv_obj_t *table_position = (lv_obj_t *)lv_event_get_user_data(e);
  if (leveling_position_buttons_map_id == -1) {
    leveling_position_buttons_map_id = 4;
  } else {
    switch (leveling_position_buttons_map_id) {
      case 0:
        leveling_position_buttons_map_id = 2;
        break;
      case 1:
        leveling_position_buttons_map_id = 2;
        break;
      case 2:
        leveling_position_buttons_map_id = 4;
        break;
      case 3:
        leveling_position_buttons_map_id = 4;
        break;
      case 4:
        leveling_position_buttons_map_id = 6;
        break;
      case 5:
        leveling_position_buttons_map_id = 6;
        break;
      case 6:
        leveling_position_buttons_map_id = 8;
        break;
      case 7:
        leveling_position_buttons_map_id = 8;
        break;
      case 8:
        leveling_position_buttons_map_id = 0;
        break;
      default:
        break;
    }
  }
  lv_btnmatrix_set_btn_ctrl(table_position, leveling_position_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  move_to_position(leveling_position_buttons_map_id);

  esp3d_log("Next Clicked");
}

void event_button_manual_leveling_previous_handler(lv_event_t *e) {
  lv_obj_t *table_position = (lv_obj_t *)lv_event_get_user_data(e);
  if (leveling_position_buttons_map_id == -1) {
    leveling_position_buttons_map_id = 4;
  } else {
    switch (leveling_position_buttons_map_id) {
      case 0:
        leveling_position_buttons_map_id = 8;
        break;
      case 1:
        leveling_position_buttons_map_id = 0;
        break;
      case 2:
        leveling_position_buttons_map_id = 0;
        break;
      case 3:
        leveling_position_buttons_map_id = 2;
        break;
      case 4:
        leveling_position_buttons_map_id = 2;
        break;
      case 5:
        leveling_position_buttons_map_id = 4;
        break;
      case 6:
        leveling_position_buttons_map_id = 4;
        break;
      case 7:
        leveling_position_buttons_map_id = 6;
        break;
      case 8:
        leveling_position_buttons_map_id = 6;
        break;
      default:
        break;
    }
  }
  lv_btnmatrix_set_btn_ctrl(table_position, leveling_position_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  move_to_position(leveling_position_buttons_map_id);
  esp3d_log("Previous Clicked");
}

void leveling_posiiton_matrix_buttons_event_cb(lv_event_t *e) {
  // lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
  lv_obj_t *obj = lv_event_get_target(e);
  uint32_t id = lv_btnmatrix_get_selected_btn(obj);
  leveling_position_buttons_map_id = id;
  esp3d_log("Button %d clicked", leveling_position_buttons_map_id);
  move_to_position(leveling_position_buttons_map_id);
}

void event_button_manual_leveling_help_handler(lv_event_t *e) {
  esp3d_log("Help Clicked");
  std::string text =
      esp3dTranslationService.translate(ESP3DLabel::manual_leveling_help);
  msgBox::create(NULL, MsgBoxType::information, text.c_str());
}

void event_button_manual_leveling_start_handler(lv_event_t *e) {
  lv_obj_t *table_position = (lv_obj_t *)lv_event_get_user_data(e);
  esp3d_log("Start Clicked");
  // send G28
  lv_obj_clear_state(btn_next, LV_STATE_DISABLED);
  lv_obj_clear_state(btn_previous, LV_STATE_DISABLED);
  lv_obj_clear_state(btnm_leveling_position, LV_STATE_DISABLED);
  lv_obj_add_flag(btn_start, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(status_container, LV_OBJ_FLAG_HIDDEN);

  // once G28 is done
  leveling_position_buttons_map_id = 4;
  lv_btnmatrix_set_btn_ctrl(table_position, leveling_position_buttons_map_id,
                            LV_BTNMATRIX_CTRL_CHECKED);
  move_to_position(leveling_position_buttons_map_id);
}
void create(bool autoleveling) {
  auto_leveling = autoleveling;
  leveling_position_buttons_map_id = -1;
  homing_done = false;
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Manual leveling screen creation");
  if (!intialization_done) {
    esp3d_log("Manual leveling screen initialization");
    char buffer[16];
    uint8_t byte_value =
        esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_inverved_x);
    invert_x(byte_value == 1 ? true : false);
    byte_value = esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_inverved_y);
    invert_y(byte_value == 1 ? true : false);
    std::string str_value = esp3dTftsettings.readString(
        ESP3DSettingIndex::esp3d_bed_width, buffer, 16);
    bed_width(std::strtod(str_value.c_str(),
                                 NULL));  // update width value
    str_value = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_bed_depth,
                                            buffer, 16);
    bed_depth(std::strtod(str_value.c_str(),
                                 NULL));  // update depth value
    intialization_done = true;
  }
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_del(ui_current_screen);

  // Button back
  lv_obj_t *btn_back = backButton::create(ui_new_screen);
  lv_obj_add_event_cb(btn_back, event_button_manual_leveling_back_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_update_layout(btn_back);

  // Manual leveling button matrix
  btnm_leveling_position = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm_leveling_position, leveling_position_buttons_map);
  ESP3DStyle::apply(btnm_leveling_position, ESP3DStyleType::buttons_matrix);
  lv_obj_update_layout(btnm_leveling_position);
  lv_obj_set_pos(btnm_leveling_position, ESP3D_BUTTON_PRESSED_OUTLINE,
                 ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_size(btnm_leveling_position, LV_HOR_RES / 2,
                  LV_VER_RES - (ESP3D_BUTTON_PRESSED_OUTLINE * 3) -
                      lv_obj_get_height(btn_back));
  lv_obj_add_event_cb(btnm_leveling_position,
                      leveling_posiiton_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  // Button Previous
  btn_previous = symbolButton::create(
      ui_new_screen, LV_SYMBOL_PREVIOUS, ESP3D_BACK_BUTTON_WIDTH, ESP3D_BACK_BUTTON_HEIGHT);
  lv_obj_align(btn_previous, LV_ALIGN_BOTTOM_LEFT,
               ESP3D_BUTTON_PRESSED_OUTLINE, -ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn_previous,
                      event_button_manual_leveling_previous_handler,
                      LV_EVENT_CLICKED, btnm_leveling_position);

  // Button Next
  btn_next = symbolButton::create(
      ui_new_screen, LV_SYMBOL_NEXT, ESP3D_BACK_BUTTON_WIDTH, ESP3D_BACK_BUTTON_HEIGHT);
  lv_obj_align_to(btn_next, btnm_leveling_position, LV_ALIGN_OUT_BOTTOM_RIGHT,
                  0, ESP3D_BUTTON_PRESSED_OUTLINE);

  lv_obj_add_event_cb(btn_next, event_button_manual_leveling_next_handler,
                      LV_EVENT_CLICKED, btnm_leveling_position);

  // Help button
  lv_obj_t *btn_help = symbolButton::create(
      ui_new_screen, "?", ESP3D_BACK_BUTTON_WIDTH, ESP3D_BACK_BUTTON_HEIGHT);
  lv_obj_align(btn_help, LV_ALIGN_TOP_RIGHT, -ESP3D_BUTTON_PRESSED_OUTLINE,
               ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn_help, event_button_manual_leveling_help_handler,
                      LV_EVENT_CLICKED, NULL);

  // Start button
  btn_start = symbolButton::create(ui_new_screen, LV_SYMBOL_PLAY);
  lv_obj_align_to(btn_start, btnm_leveling_position, LV_ALIGN_OUT_RIGHT_MID,
                  (LV_HOR_RES / 4) - ESP3D_SYMBOL_BUTTON_WIDTH / 2, 0);
  lv_obj_add_event_cb(btn_start, event_button_manual_leveling_start_handler,
                      LV_EVENT_CLICKED, btnm_leveling_position);

  // Status container
  status_container = lv_obj_create(ui_new_screen);

  ESP3DStyle::apply(status_container, ESP3DStyleType::text_container);
  lv_obj_set_size(status_container,
                  (LV_HOR_RES / 2) - (3 * ESP3D_BUTTON_PRESSED_OUTLINE),
                  LV_VER_RES - (ESP3D_BUTTON_PRESSED_OUTLINE * 4) -
                      lv_obj_get_height(btn_back) * 2);
  lv_obj_align_to(status_container, btn_help, LV_ALIGN_OUT_BOTTOM_RIGHT, 0,
                  ESP3D_BUTTON_PRESSED_OUTLINE);

  // status text
  std::string t =
      esp3dTranslationService.translate(ESP3DLabel::manual_leveling_text);
  lv_obj_t *status_text = lv_label_create(status_container);
  lv_obj_set_width(status_text, lv_obj_get_content_width(status_container));
  lv_label_set_long_mode(status_text, LV_LABEL_LONG_WRAP);
  lv_label_set_text(status_text, t.c_str());

  // Init state
  lv_obj_add_state(btn_next, LV_STATE_DISABLED);
  lv_obj_add_state(btn_previous, LV_STATE_DISABLED);
  lv_obj_add_state(btnm_leveling_position, LV_STATE_DISABLED);
  lv_obj_add_flag(status_container, LV_OBJ_FLAG_HIDDEN);

  esp3dTftui.set_current_screen(ESP3DScreenType::manual_leveling);
}

}  // namespace manualLevelingScreen