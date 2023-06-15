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
#include "lvgl.h"
#include "version.h"

#define LV_TICK_PERIOD_MS 10
/**********************
 *  STATIC PROTOTYPES
 **********************/
// Create style for main screen container
lv_style_t style_container_main_screen;
// Create style for buttons line container
lv_style_t style_buttons_container;
// Create style for buttons
lv_style_t style_button;

lv_obj_t *status_bar(lv_obj_t *screen, lv_obj_t *page_container);
size_t status_bar_height();
#define BUTTON_RADIUS 10

void main_screen() {
  // Screen creation
  esp3d_log_e("Main screen creation");
  lv_obj_t *ui_main_screen = lv_obj_create(NULL);
  // Apply background color
  lv_obj_set_style_bg_color(ui_main_screen, lv_color_hex(0x000000),
                            LV_PART_MAIN);
  lv_obj_clear_flag(ui_main_screen, LV_OBJ_FLAG_SCROLLABLE);

  // Create style for main screen container
  lv_style_init(&style_container_main_screen);
  // the container will be flex column
  lv_style_set_layout(&style_container_main_screen, LV_LAYOUT_FLEX);
  lv_style_set_flex_flow(&style_container_main_screen, LV_FLEX_FLOW_COLUMN);
  lv_style_set_flex_main_place(&style_container_main_screen,
                               LV_FLEX_ALIGN_SPACE_EVENLY);

  // Create style for main screen buttons line
  lv_style_init(&style_buttons_container);
  // the container will be flex row
  lv_style_set_layout(&style_buttons_container, LV_LAYOUT_FLEX);
  lv_style_set_flex_flow(&style_buttons_container, LV_FLEX_FLOW_ROW);
  lv_style_set_flex_main_place(&style_buttons_container,
                               LV_FLEX_ALIGN_SPACE_EVENLY);

  // Create style for buttons
  lv_style_init(&style_button);

  // create container for main screen buttons
  lv_obj_t *ui_container_main_screen = lv_obj_create(ui_main_screen);

  // remove all styles from the container object
  lv_obj_remove_style_all(ui_container_main_screen);
  // lv_obj_clear_flag(ui_container_main_screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_style(ui_container_main_screen, &style_container_main_screen,
                   LV_PART_MAIN);
  lv_obj_set_style_pad_left(ui_container_main_screen, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(ui_container_main_screen, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(ui_container_main_screen, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(ui_container_main_screen, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_row(ui_container_main_screen, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_column(ui_container_main_screen, 0, LV_PART_MAIN);

  lv_obj_t *status_label = status_bar(ui_main_screen, ui_container_main_screen);
  esp3d_log_e("get status_bar: %d", (int)status_label);
  // Set container size
  lv_obj_set_size(ui_container_main_screen, LV_HOR_RES,
                  LV_VER_RES - status_bar_height());

  // Align container under status bar
  lv_obj_align_to(ui_container_main_screen, status_label,
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  // create container for main screen buttons Line 1
  lv_obj_t *line1_buttons_container = lv_obj_create(ui_container_main_screen);
  // remove all styles from the container object
  lv_obj_remove_style_all(line1_buttons_container);
  lv_style_set_bg_color(&style_buttons_container, lv_color_hex(0xFF0000));
  lv_style_set_bg_opa(&style_buttons_container, LV_OPA_COVER);
  lv_style_set_radius(&style_buttons_container, BUTTON_RADIUS);
  lv_obj_add_style(line1_buttons_container, &style_buttons_container,
                   LV_PART_MAIN);
  // use all space as width
  lv_obj_set_width(line1_buttons_container, LV_HOR_RES);
  // use all space as height
  lv_obj_set_height(line1_buttons_container, LV_SIZE_CONTENT);

  // Add buttons to line 1
  lv_obj_t *btn1 = lv_btn_create(line1_buttons_container);
  lv_obj_t *label1 = lv_label_create(btn1);
  lv_label_set_text(label1, "200\n200\n" LV_SYMBOL_WIZARD);
  lv_obj_set_align(label1, LV_ALIGN_CENTER);
  lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_t *btn2 = lv_btn_create(line1_buttons_container);
  lv_obj_t *label2 = lv_label_create(btn2);
  lv_label_set_text(label2, "60\n60\n" LV_SYMBOL_LANGUAGE);
  lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_center(label2);
  /*
    // create container for main screen buttons Line 2
    lv_obj_t *line2_buttons_container = lv_obj_create(ui_container_main_screen);
    // remove all styles from the container object
    lv_obj_remove_style_all(line2_buttons_container);
    lv_obj_add_style(line2_buttons_container, &style_buttons_container,
                     LV_PART_MAIN);
    // use all space as width
    lv_obj_set_width(line2_buttons_container, LV_HOR_RES);
    // use all space as height
    lv_obj_set_height(line2_buttons_container, lv_pct(100));
  */
  // Add buttons to line 2
  /*lv_obj_t *btn1_2 = lv_btn_create(line2_buttons_container);
  lv_obj_t *label1_2 = lv_label_create(btn1_2);
  lv_label_set_text(label1_2, "200\n200\n" LV_SYMBOL_BLUETOOTH);
  lv_obj_center(label1_2);
  lv_obj_t *btn2_2 = lv_btn_create(line2_buttons_container);
  lv_obj_t *label2_2 = lv_label_create(btn2_2);
  lv_label_set_text(label2_2, "60\n60\n" LV_SYMBOL_BLUETOOTH);
  lv_obj_center(label2_2);*/
  /*
    // create container for main screen buttons Line 3
    lv_obj_t *line3_buttons_container = lv_obj_create(ui_container_main_screen);
    // remove all styles from the container object
    lv_obj_remove_style_all(line3_buttons_container);
    lv_obj_add_style(line3_buttons_container, &style_buttons_container,
                     LV_PART_MAIN);
    // use all space as width
    lv_obj_set_width(line3_buttons_container, LV_HOR_RES);
    // use all space as height
    lv_obj_set_height(line3_buttons_container, lv_pct(100));
  */
  // Add buttons to line 3
  /*lv_obj_t *btn1_3 = lv_btn_create(line3_buttons_container);
  lv_obj_t *label1_3 = lv_label_create(btn1_3);
  lv_label_set_text(label1_3, LV_SYMBOL_SETTINGS);
  lv_obj_center(label1_3);
  lv_obj_t *btn2_3 = lv_btn_create(line3_buttons_container);
  lv_obj_t *label2_3 = lv_label_create(btn2_3);
  lv_label_set_text(label2_3, LV_SYMBOL_WIFI);
  lv_obj_center(label2_3);*/

  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_main_screen);
  lv_obj_del(ui_current_screen);
}
