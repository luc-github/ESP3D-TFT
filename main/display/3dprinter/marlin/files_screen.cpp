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

#include "files_screen.h"

#include <list>

#include "back_button_component.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "list_line_component.h"
#include "main_screen.h"
#include "symbol_button_component.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace filesScreen {

lv_timer_t *files_screen_delay_timer = NULL;
lv_obj_t *ui_files_list_ctl = NULL;
lv_obj_t *files_spinner = NULL;
std::string files_path = "/mypath/from/esp3d";
struct ESP3DFileDescriptor {
  std::string name;
  std::string size;
};
std::list<ESP3DFileDescriptor> files_list;

void event_button_files_up_handler(lv_event_t *e) {
  int pos = esp3d_strings::rfind(files_path.c_str(), "/", -1);
  esp3d_log("up Clicked %d", pos);
  std::string newpath = files_path.substr(0, pos);
  if (newpath == "") newpath = "/";
  esp3d_log("old path %s, new path %s", files_path.c_str(), newpath.c_str());
  files_path = newpath;
  files_screen();
}

void files_screen_delay_timer_cb(lv_timer_t *timer) {
  if (files_screen_delay_timer) {
    lv_timer_del(files_screen_delay_timer);
    files_screen_delay_timer = NULL;
  }
  mainScreen::main_screen();
}

void event_button_files_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (files_screen_delay_timer) return;
    files_screen_delay_timer = lv_timer_create(files_screen_delay_timer_cb,
                                               BUTTON_ANIMATION_DELAY, NULL);
  } else {
    files_screen_delay_timer_cb(NULL);
  }
}

void event_button_files_refresh_handler(lv_event_t *e) {
  esp3d_log("refresh Clicked");
  lv_obj_clear_flag(files_spinner, LV_OBJ_FLAG_HIDDEN);
  size_t i = lv_obj_get_child_cnt(ui_files_list_ctl);
  while (i > 0) {
    lv_obj_del(lv_obj_get_child(ui_files_list_ctl, 0));
    i--;
  }
}

void files_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Files screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);

  // button back
  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_files_back_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_update_layout(btnback);

  // button refresh
  lv_obj_t *btnrefresh = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_REFRESH, SYMBOL_BUTTON_WIDTH,
      lv_obj_get_height(btnback));
  lv_obj_align(btnrefresh, LV_ALIGN_BOTTOM_MID, 0,
               -CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(btnrefresh, event_button_files_refresh_handler,
                      LV_EVENT_CLICKED, NULL);
  // label path
  lv_obj_t *labelpath = lv_label_create(ui_new_screen);
  lv_label_set_text(labelpath, files_path.c_str());
  lv_label_set_long_mode(labelpath, LV_LABEL_LONG_SCROLL_CIRCULAR);
  apply_style(labelpath, ESP3DStyleType::bg_label);
  lv_obj_set_pos(labelpath, CURRENT_STATUS_BAR_V_PAD, CURRENT_STATUS_BAR_V_PAD);
  lv_obj_set_width(labelpath, LV_HOR_RES - (2 * CURRENT_STATUS_BAR_V_PAD));
  lv_obj_set_style_pad_left(labelpath, CURRENT_BUTTON_PRESSED_OUTLINE,
                            LV_PART_MAIN);
  lv_obj_set_style_pad_right(labelpath, CURRENT_BUTTON_PRESSED_OUTLINE,
                             LV_PART_MAIN);
  lv_label_set_long_mode(labelpath, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_update_layout(labelpath);

  // list control
  ui_files_list_ctl = lv_list_create(ui_new_screen);
  lv_obj_clear_flag(ui_files_list_ctl, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_pos(ui_files_list_ctl, 0,
                 lv_obj_get_height(labelpath) + (2 * CURRENT_STATUS_BAR_V_PAD));
  lv_obj_set_size(
      ui_files_list_ctl, LV_HOR_RES,
      LV_VER_RES -
          ((1.5 * CURRENT_BUTTON_PRESSED_OUTLINE) + lv_obj_get_height(btnback) +
           lv_obj_get_height(labelpath) + (2 * CURRENT_STATUS_BAR_V_PAD)));

  // spinner creation in center of screen
  files_spinner = lv_spinner_create(ui_new_screen, 1000, 60);
  lv_obj_set_size(files_spinner, CURRENT_SPINNER_SIZE, CURRENT_SPINNER_SIZE);
  lv_obj_center(files_spinner);
  lv_obj_add_flag(files_spinner, LV_OBJ_FLAG_HIDDEN);

  if (files_path != "/") {
    std::string tmplabel = ".." LV_SYMBOL_NEW_LINE;
    lv_obj_t *line1 = listLine::create_list_line_container(ui_files_list_ctl);
    lv_obj_t *btn_prev = listLine::add_button_to_line(tmplabel.c_str(), line1);
    lv_obj_add_event_cb(btn_prev, event_button_files_up_handler,
                        LV_EVENT_CLICKED, NULL);
  }

  for (uint8_t i = 0; i < 20; i++) {
    lv_obj_t *line_container =
        listLine::create_list_line_container(ui_files_list_ctl);
    lv_obj_t *h =
        listLine::add_label_to_line(LV_SYMBOL_FOLDER, line_container, false);
    lv_obj_t *n =
        listLine::add_label_to_line("Directory name", line_container, true);
    lv_obj_t *btn =
        listLine::add_button_to_line(LV_SYMBOL_SEARCH, line_container);
  }

  esp3dTftui.set_current_screen(ESP3DScreenType::files);
}

}  // namespace filesScreen