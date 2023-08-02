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
#include "filesystem/esp3d_sd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "list_line_component.h"
#include "main_screen.h"
#include "symbol_button_component.h"
#include "tasks_def.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace filesScreen {
#define STACKDEPTH 4096
#define TASKPRIORITY UI_TASK_PRIORITY - 1
#define TASKCORE UI_TASK_CORE

lv_timer_t *files_screen_delay_timer = NULL;
lv_obj_t *refresh_button = NULL;
lv_obj_t *ui_files_list_ctl = NULL;
lv_obj_t *files_spinner = NULL;
lv_timer_t *start_files_list_timer = NULL;
std::string files_path = "/";
struct ESP3DFileDescriptor {
  std::string name;
  std::string size;
};
std::list<ESP3DFileDescriptor> files_list;
void fill_files_list();
void refresh_files_list_cb(lv_timer_t *timer) {
  if (start_files_list_timer) {
    lv_timer_del(start_files_list_timer);
    start_files_list_timer = NULL;
  }
  files_screen();
}
static void bgFilesTask(void *pvParameter) {
  (void)pvParameter;
  vTaskDelay(pdMS_TO_TICKS(100));
  fill_files_list();
  if (!start_files_list_timer) {
    start_files_list_timer = lv_timer_create(refresh_files_list_cb, 100, NULL);
  }
  vTaskDelete(NULL);
}

bool playable_file(const char *name) {
  int pos = esp3d_strings::rfind(name, ".", -1);
  if (pos != -1) {
    std::string ext = name + pos;
    esp3d_strings::str_toLowerCase(&ext);
    // TODO:
    //  use the preferences.json list instead of hard coded list
    if (ext == ".gcode" || ext == ".gco" || ext == ".g" || ext == ".gc") {
      return true;
    }
  }
  return false;
}

void fill_files_list() {
  files_list.clear();
  if (sd.accessFS()) {
    DIR *dir = sd.opendir(files_path.c_str());
    if (dir) {
      struct dirent *ent;
      struct stat entry_stat;
      while ((ent = readdir(dir)) != NULL) {
        ESP3DFileDescriptor file;
        if (ent->d_type == DT_DIR) {
          if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
          file.name = ent->d_name;
          file.size = "-1";
          files_list.push_back(file);
        } else {
          if (!playable_file(ent->d_name)) continue;
          file.name = ent->d_name;
          file.size = "0";
          std::string fullPath = "/";
          if (files_path != "/") {
            fullPath = files_path + std::string("/") + ent->d_name;
          } else {
            fullPath += ent->d_name;
          }
          if (sd.stat(fullPath.c_str(), &entry_stat) == -1) {
            esp3d_log_e("Failed to stat %s : %s",
                        ent->d_type == DT_DIR ? "DIR" : "FILE", ent->d_name);
            continue;
          }
          esp3d_log("File size is %ld", entry_stat.st_size);
          file.size = esp3d_strings::formatBytes(entry_stat.st_size);
          files_list.push_back(file);
        }
        esp3d_log("Found %s, %s", file.name.c_str(), file.size.c_str());
      }
      esp3d_log("Files list size %d", files_list.size());
      closedir(dir);
    }
    sd.releaseFS();
  }
}

void do_files_list_now() {
  TaskHandle_t xHandle = NULL;
  BaseType_t res =
      xTaskCreatePinnedToCore(bgFilesTask, "filesTask", STACKDEPTH, NULL,
                              TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Files Task");
  } else {
    esp3d_log_e("Files Task creation failed");
  }
}

void start_files_list_timer_cb(lv_timer_t *timer) {
  if (start_files_list_timer) {
    lv_timer_del(start_files_list_timer);
    start_files_list_timer = NULL;
  }
  do_files_list_now();
}

void event_button_files_refresh_handler(lv_event_t *e) {
  esp3d_log("refresh Clicked");
  lv_obj_clear_flag(files_spinner, LV_OBJ_FLAG_HIDDEN);
  size_t i = lv_obj_get_child_cnt(ui_files_list_ctl);
  while (i > 0) {
    lv_obj_del(lv_obj_get_child(ui_files_list_ctl, 0));
    i--;
  }
  lv_obj_clear_flag(files_spinner, LV_OBJ_FLAG_HIDDEN);
  if (!start_files_list_timer) {
    start_files_list_timer =
        lv_timer_create(start_files_list_timer_cb, 100, NULL);
  }
}

void event_button_files_up_handler(lv_event_t *e) {
  int pos = esp3d_strings::rfind(files_path.c_str(), "/", -1);
  esp3d_log("up Clicked %d", pos);
  std::string newpath = files_path.substr(0, pos);
  if (newpath == "") newpath = "/";
  esp3d_log("old path %s, new path %s", files_path.c_str(), newpath.c_str());
  files_path = newpath;
  event_button_files_refresh_handler(e);
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

void event_directory_handler(lv_event_t *e) {
  ESP3DFileDescriptor *file = (ESP3DFileDescriptor *)lv_event_get_user_data(e);

  esp3d_log("dir Clicked: %s", file->name.c_str());
  if (files_path != "/") {
    files_path += std::string("/") + file->name;
  } else {
    files_path += file->name;
  }
  event_button_files_refresh_handler(e);
}

void event_file_handler(lv_event_t *e) {
  ESP3DFileDescriptor *file = (ESP3DFileDescriptor *)lv_event_get_user_data(e);

  esp3d_log("file Clicked: %s", file->name.c_str());
  std::string file_path_to_play = files_path;
  if (file_path_to_play != "/") {
    file_path_to_play = std::string("/") + file->name;
  } else {
    file_path_to_play = file->name;
  }
  // todo play file and go to main screen
  esp3dTftValues.set_string_value(ESP3DValuesIndex::file_path,
                                  file_path_to_play.c_str());
  esp3dTftValues.set_string_value(ESP3DValuesIndex::file_name,
                                  file->name.c_str());
  esp3dTftValues.set_string_value(ESP3DValuesIndex::print_status, "printing");
  mainScreen::main_screen();
}

bool first_fill_needed = true;
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
  refresh_button = symbolButton::create_symbol_button(
      ui_new_screen, LV_SYMBOL_REFRESH, SYMBOL_BUTTON_WIDTH,
      lv_obj_get_height(btnback));
  lv_obj_align(refresh_button, LV_ALIGN_BOTTOM_MID, 0,
               -CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(refresh_button, event_button_files_refresh_handler,
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
  // populate if not done
  if (first_fill_needed) {
    if (files_list.size() == 0) {
      event_button_files_refresh_handler(nullptr);
    }
    first_fill_needed = false;
  }

  // dir first
  for (auto &file : files_list) {
    if (file.size == "-1") {
      lv_obj_t *line_container =
          listLine::create_list_line_container(ui_files_list_ctl);
      listLine::add_label_to_line(LV_SYMBOL_FOLDER, line_container, false);
      listLine::add_label_to_line(file.name.c_str(), line_container, true);
      lv_obj_t *btn =
          listLine::add_button_to_line(LV_SYMBOL_SEARCH, line_container);
      lv_obj_add_event_cb(btn, event_directory_handler, LV_EVENT_CLICKED,
                          &file);
    }
  }

  for (auto &file : files_list) {
    if (file.size != "-1") {
      lv_obj_t *line_container =
          listLine::create_list_line_container(ui_files_list_ctl);
      listLine::add_label_to_line(LV_SYMBOL_FILE, line_container, false);
      listLine::add_label_to_line(file.name.c_str(), line_container, true);
      listLine::add_label_to_line(file.size.c_str(), line_container, false);
      lv_obj_t *btn =
          listLine::add_button_to_line(LV_SYMBOL_PLAY, line_container);
      lv_obj_add_event_cb(btn, event_file_handler, LV_EVENT_CLICKED, &file);
    }
  }

  esp3dTftui.set_current_screen(ESP3DScreenType::files);
}

}  // namespace filesScreen