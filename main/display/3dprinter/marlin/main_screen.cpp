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

#include <string>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

lv_obj_t *status_bar(lv_obj_t *screen);

void main_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::main);
  // Screen creation
  esp3d_log("Main screen creation");
  // Background
  lv_obj_t *ui_main_screen = lv_obj_create(NULL);
  apply_style(ui_main_screen, ESP3DStyleType::main_bg);

  lv_obj_t *ui_status_bar_container = status_bar(ui_main_screen);
  lv_obj_update_layout(ui_status_bar_container);
  // create container for main screen buttons

  lv_obj_t *ui_container_main_screen = lv_obj_create(ui_main_screen);
  apply_style(ui_container_main_screen, ESP3DStyleType::col_container);
  lv_obj_clear_flag(ui_container_main_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Set container size
  lv_obj_set_size(ui_container_main_screen, LV_HOR_RES,
                  LV_VER_RES - lv_obj_get_height(ui_status_bar_container));
  // Align container under status bar
  lv_obj_align_to(ui_container_main_screen, ui_status_bar_container,
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  // Add buttons containers to main container
  lv_obj_t *ui_heaters_container = lv_obj_create(ui_container_main_screen);
  apply_style(ui_heaters_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_heaters_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_heaters_container);

  lv_obj_t *ui_move_container = lv_btn_create(ui_container_main_screen);
  apply_style(ui_move_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_move_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_move_container);
  lv_obj_t *ui_btn_move = lv_btn_create(ui_move_container);
  apply_style(ui_btn_move, ESP3DStyleType::button);

  lv_obj_t *ui_sensors_container = lv_obj_create(ui_container_main_screen);
  apply_style(ui_sensors_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_sensors_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_sensors_container);

  lv_obj_t *ui_menu_container = lv_obj_create(ui_container_main_screen);
  apply_style(ui_menu_container, ESP3DStyleType::row_container);
  lv_obj_set_size(ui_menu_container, LV_HOR_RES, LV_SIZE_CONTENT);
  apply_outline_pad(ui_menu_container);

#define BUTTON_WIDTH (LV_HOR_RES / 6)

  //**********************************
  lv_obj_t *btn1 = lv_btn_create(ui_heaters_container);
  apply_style(btn1, ESP3DStyleType::button);
  lv_obj_set_size(btn1, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_t *label = lv_label_create(btn1);
  lv_label_set_text_fmt(label, "%s\n%s\n%s%s", "160", "260", LV_SYMBOL_EXTRUDER,
                        "1");
  lv_obj_center(label);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *btn2 = lv_btn_create(ui_heaters_container);
  apply_style(btn2, ESP3DStyleType::button);
  lv_obj_set_size(btn2, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_t *label2 = lv_label_create(btn2);
  lv_label_set_text_fmt(label2, "%s\n%s\n%s%s", "60", "60", LV_SYMBOL_EXTRUDER,
                        "2");
  lv_obj_center(label2);
  lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *btn3 = lv_btn_create(ui_heaters_container);
  apply_style(btn3, ESP3DStyleType::button);
  lv_obj_set_size(btn3, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_t *label3 = lv_label_create(btn3);
  lv_label_set_text_fmt(label3, "%s\n%s\n%s", "60", "60", LV_SYMBOL_HEAT_BED);
  lv_obj_center(label3);
  lv_obj_set_style_text_align(label3, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *posxyz = lv_label_create(ui_btn_move);
  lv_label_set_text_fmt(posxyz, "    X: %s     Y: %s     Z: %s    ", "100.0",
                        "160.0", "1.0");
  lv_obj_center(posxyz);
  lv_obj_set_size(ui_btn_move, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

  lv_obj_t *btnsensors1 = lv_btn_create(ui_sensors_container);
  apply_style(btnsensors1, ESP3DStyleType::button);
  lv_obj_set_size(btnsensors1, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_t *labelsensors1 = lv_label_create(btnsensors1);
  lv_label_set_text_fmt(labelsensors1, "%s\n%s", "100%", LV_SYMBOL_FAN);
  lv_obj_center(labelsensors1);
  lv_obj_set_style_text_align(labelsensors1, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *btnsensors2 = lv_btn_create(ui_sensors_container);
  apply_style(btnsensors2, ESP3DStyleType::button);
  lv_obj_set_size(btnsensors2, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_t *labelsensors2 = lv_label_create(btnsensors2);
  lv_label_set_text_fmt(labelsensors2, "%s\n%s", "100%", LV_SYMBOL_SPEED);
  lv_obj_center(labelsensors2);
  lv_obj_set_style_text_align(labelsensors2, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *btnsensors3 = lv_btn_create(ui_sensors_container);
  apply_style(btnsensors3, ESP3DStyleType::button);
  lv_obj_set_size(btnsensors3, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_t *labelsensors3 = lv_label_create(btnsensors3);
  lv_label_set_text_fmt(labelsensors3, "%s\n%s", "40%", LV_SYMBOL_PAUSE);
  lv_obj_center(labelsensors3);
  lv_obj_set_style_text_align(labelsensors3, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *btnsensors4 = lv_btn_create(ui_sensors_container);
  apply_style(btnsensors4, ESP3DStyleType::button);
  lv_obj_set_size(btnsensors4, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_t *labelsensors4 = lv_label_create(btnsensors4);
  lv_label_set_text(labelsensors4, LV_SYMBOL_STOP);
  lv_obj_center(labelsensors4);
  lv_obj_set_style_text_align(labelsensors4, LV_TEXT_ALIGN_CENTER, 0);

  lv_obj_t *btnsensors5 = lv_btn_create(ui_sensors_container);
  apply_style(btnsensors5, ESP3DStyleType::button);
  lv_obj_set_size(btnsensors5, BUTTON_WIDTH, BUTTON_WIDTH);
  lv_obj_t *labelsensors5 = lv_label_create(btnsensors5);
  lv_label_set_text(labelsensors5, LV_SYMBOL_SETTINGS);
  lv_obj_center(labelsensors5);
  lv_obj_set_style_text_align(labelsensors5, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_clear_flag(ui_sensors_container, LV_OBJ_FLAG_SCROLLABLE);

  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_main_screen);
  lv_obj_del(ui_current_screen);
}
