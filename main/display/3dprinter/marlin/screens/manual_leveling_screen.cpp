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

#include "manual_leveling_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "leveling_screen.h"
#include "menu_screen.h"

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

lv_timer_t *manual_leveling_screen_delay_timer = NULL;

void manual_leveling_screen_delay_timer_cb(lv_timer_t *timer) {
  if (manual_leveling_screen_delay_timer) {
    lv_timer_del(manual_leveling_screen_delay_timer);
    manual_leveling_screen_delay_timer = NULL;
  }
  if (auto_leveling) {
    levelingScreen::leveling_screen(auto_leveling);
  } else {
    menuScreen::menu_screen();
  }
}

void event_button_manual_leveling_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (manual_leveling_screen_delay_timer) return;
    manual_leveling_screen_delay_timer = lv_timer_create(
        manual_leveling_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    manual_leveling_screen_delay_timer_cb(NULL);
  }
}

void move_to_position(int pos) {
  if (!homing_done) {
    esp3d_log("Homing not done, doing now...");
    // Todo : send G28

    homing_done = true;
  }
  switch (pos) {
    case 0:
      // Move to position 0
      esp3d_log("Move to position 0");
      break;
    case 1:
      // Move to position 1
      esp3d_log("Move to position 1");
      break;
    case 2:
      // Move to position 2
      esp3d_log("Move to position 2");
      break;
    case 3:
      // Move to position 3
      esp3d_log("Move to position 3");
      break;
    case 4:
      // Move to position 4
      esp3d_log("Move to position 4");
      break;
    case 5:
      // Move to position 5
      esp3d_log("Move to position 5");
      break;
    case 6:
      // Move to position 6
      esp3d_log("Move to position 6");
      break;
    case 7:
      // Move to position 7
      esp3d_log("Move to position 7");
      break;
    case 8:
      // Move to position 8
      esp3d_log("Move to position 8");
      break;
    default:
      break;
  }
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

void manual_leveling_screen(bool autoleveling) {
  auto_leveling = autoleveling;
  leveling_position_buttons_map_id = -1;
  homing_done = false;
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Manual leveling screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  // Button back
  lv_obj_t *btn_back = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btn_back, event_button_manual_leveling_back_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_update_layout(btn_back);

  // Manual leveling button matrix
  btnm_leveling_position = lv_btnmatrix_create(ui_new_screen);
  lv_btnmatrix_set_map(btnm_leveling_position, leveling_position_buttons_map);
  apply_style(btnm_leveling_position, ESP3DStyleType::buttons_matrix);
  lv_obj_update_layout(btnm_leveling_position);
  lv_obj_set_pos(btnm_leveling_position, CURRENT_BUTTON_PRESSED_OUTLINE,
                 CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_size(btnm_leveling_position, LV_HOR_RES / 2,
                  LV_VER_RES - (CURRENT_BUTTON_PRESSED_OUTLINE * 3) -
                      lv_obj_get_height(btn_back));
  lv_obj_add_event_cb(btnm_leveling_position,
                      leveling_posiiton_matrix_buttons_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  // Button Previous
  lv_obj_t *btn_previous = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_PREVIOUS, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT);
  lv_obj_align(btn_previous, LV_ALIGN_BOTTOM_LEFT,
               CURRENT_BUTTON_PRESSED_OUTLINE, -CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btn_previous,
                      event_button_manual_leveling_previous_handler,
                      LV_EVENT_CLICKED, btnm_leveling_position);

  // Button Next
  lv_obj_t *btn_next = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_NEXT, BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT);
  lv_obj_align_to(btn_next, btnm_leveling_position, LV_ALIGN_OUT_BOTTOM_RIGHT,
                  0, CURRENT_BUTTON_PRESSED_OUTLINE);

  lv_obj_add_event_cb(btn_next, event_button_manual_leveling_next_handler,
                      LV_EVENT_CLICKED, btnm_leveling_position);

  esp3dTftui.set_current_screen(ESP3DScreenType::manual_leveling);
}

}  // namespace manualLevelingScreen