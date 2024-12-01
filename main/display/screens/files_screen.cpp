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
#if ESP3D_SD_CARD_FEATURE
#include "screens/files_screen.h"

#include <lvgl.h>

#include <list>
#include <vector>

#include "components/back_button_component.h"
#include "components/list_line_component.h"
#include "components/spinner_component.h"
#include "components/symbol_button_component.h"
#include "esp3d_hal.h"
#include "esp3d_json_settings.h"
#include "esp3d_log.h"
#include "esp3d_lvgl.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "filesystem/esp3d_sd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gcode_host/esp3d_gcode_host_service.h"
#include "screens/main_screen.h"
#include "tasks_def.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  Namespace
 **********************/
namespace filesScreen {
#define STACKDEPTH 4096
#define TASKPRIORITY UI_TASK_PRIORITY - 1
#define TASKCORE UI_TASK_CORE
// static variables
bool first_fill_needed = true;
lv_timer_t *files_screen_delay_timer = NULL;
lv_obj_t *refresh_button = NULL;
lv_obj_t *ui_files_list_ctl = NULL;
lv_timer_t *start_files_list_timer = NULL;
lv_obj_t *msg = NULL;
bool files_has_sd = false;
std::string files_path = "/";
std::vector<std::string> files_extensions;
struct ESP3DFileDescriptor {
  std::string name;
  std::string size;
};
std::list<ESP3DFileDescriptor> files_list;
void fill_files_list();
void create();

void refresh_files_list_cb(lv_timer_t *timer) {
  if (start_files_list_timer) {
    lv_timer_del(start_files_list_timer);
    start_files_list_timer = NULL;
  }
  spinnerScreen::hide();
  create();
}
/**
 * @brief Background task for handling files screen.
 *
 * This task is responsible for filling the files list and creating a timer for
 * refreshing the list.
 *
 * @param pvParameter Pointer to the task parameter (not used).
 */
static void bgFilesTask(void *pvParameter) {
  (void)pvParameter;
  esp3d_hal::wait(100);
  fill_files_list();
  if (!start_files_list_timer) {
    start_files_list_timer = lv_timer_create(refresh_files_list_cb, 100, NULL);
    if (!start_files_list_timer) {
      esp3d_log_e("Failed to create timer");
    }
  }
  vTaskDelete(NULL);
}

/**
 * Checks if a file is playable based on its extension.
 *
 * @param name The name of the file.
 * @return True if the file is playable, false otherwise.
 */
bool playable_file(const char *name) {
  if (files_extensions.size() == 0) return true;
  int pos = esp3d_string::rfind(name, ".", -1);
  if (pos != -1) {
    std::string ext = name + pos + 1;
    esp3d_string::str_toLowerCase(&ext);
    for (auto &extension : files_extensions) {
      if (ext == extension) {
        return true;
      }
    }
  }
  return false;
}

/**
 * Checks if a given name is present in the list of file extensions.
 *
 * @param name The name to check.
 * @return True if the name is found in the list of file extensions, false
 * otherwise.
 */
bool is_in_list(const char *name) {
  for (auto &ext : files_extensions) {
    if (ext == name) return true;
  }
  return false;
}

/**
 * @brief Clears the files list and fills it with file information from the SD
 * card.
 *
 * This function clears the files list and then populates it with information
 * about the files present on the SD card. It reads the file extensions filter
 * from the settings, converts it to lowercase, and adds each extension to the
 * files_extensions list. Then, it checks if the SD card is accessible and opens
 * the directory specified by files_path. For each file or directory in the
 * directory, it creates an ESP3DFileDescriptor object and adds it to the
 * files_list. If the file is a directory, it sets the size to "-1". If the file
 * is a playable file, it sets the size to "0" and retrieves the actual file
 * size using the stat function. The file size is then formatted using the
 * formatBytes function and added to the ESP3DFileDescriptor object. Finally,
 * the function logs the name and size of each file found and the total size of
 * the files list.
 *
 * @note This function assumes that the sd object and other necessary variables
 * have been properly initialized.
 */
void fill_files_list() {
  files_list.clear();
  std::string extensionvalues =
      esp3dTftJsonSettings.readString("settings", "filesfilter");
  files_extensions.clear();
  esp3d_string::str_toLowerCase(&extensionvalues);
  if (extensionvalues.length() > 0) {
    char str[extensionvalues.length() + 1];
    char *p;
    strcpy(str, extensionvalues.c_str());
    p = strtok(str, ";");
    while (p != NULL) {
      std::string ext = p;
      if (!is_in_list(ext.c_str())) {
        esp3d_log("Add extension %s", ext.c_str());
        files_extensions.push_back(ext);
      }
      p = strtok(NULL, ";");
    }
  }
  if (sd.accessFS()) {
    files_has_sd = true;
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
          file.size = esp3d_string::formatBytes(entry_stat.st_size);
          files_list.push_back(file);
        }
        esp3d_log("Found %s, %s", file.name.c_str(), file.size.c_str());
        esp3d_hal::wait(1);
      }
      esp3d_log("Files list size %d", files_list.size());
      closedir(dir);
    }
    sd.releaseFS();
  } else {
    files_has_sd = false;
  }
}

/**
 * @brief Performs the files list operation.
 *
 * This function shows a spinner screen, clears the message label, and creates a
 * task to perform the files list operation in the background. If the task
 * creation is successful, it logs a message indicating the creation of the
 * task. If the task creation fails, it hides the spinner screen and logs an
 * error message.
 */
void do_files_list_now() {
  spinnerScreen::show();
  if (msg) lv_label_set_text(msg, "");
  TaskHandle_t xHandle = NULL;
  BaseType_t res =
      xTaskCreatePinnedToCore(bgFilesTask, "filesTask", STACKDEPTH, NULL,
                              TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Files Task");
  } else {
    spinnerScreen::hide();
    esp3d_log_e("Scan Task creation failed %d , %d", (int)res, (int)xHandle);
  }
}

/**
 * @brief Event handler for the "refresh" button in the files screen.
 *
 * This function is called when the "refresh" button is clicked. It logs a
 * message indicating that the button has been clicked, and then calls the
 * `do_files_list_now()` function to refresh the files list.
 *
 * @param e Pointer to the event object.
 */
void event_button_files_refresh_handler(lv_event_t *e) {
  esp3d_log("refresh Clicked");

  do_files_list_now();
}

/**
 * Handles the event when the "up" button is clicked in the files screen.
 *
 * This function updates the `files_path` variable by removing the last
 * directory from the path. If the resulting path is empty, it sets it to "/".
 * It then calls the `event_button_files_refresh_handler` function to refresh
 * the files screen.
 *
 * @param e The pointer to the event object.
 */
void event_button_files_up_handler(lv_event_t *e) {
  int pos = esp3d_string::rfind(files_path.c_str(), "/", -1);
  esp3d_log("up Clicked %d", pos);
  std::string newpath = files_path.substr(0, pos);
  if (newpath == "") newpath = "/";
  esp3d_log("old path %s, new path %s", files_path.c_str(), newpath.c_str());
  files_path = newpath;
  event_button_files_refresh_handler(e);
}

void files_screen_delay_timer_cb(lv_timer_t *timer) {
  if (files_screen_delay_timer && lv_timer_is_valid(files_screen_delay_timer)) {
    lv_timer_del(files_screen_delay_timer);
  }
  files_screen_delay_timer = NULL;
  mainScreen::create();
}

/**
 * @brief Event handler for the "back" button in the files screen.
 *
 * This function is called when the "back" button is clicked in the files
 * screen. It logs a message and handles the button animation delay if enabled.
 *
 * @param e Pointer to the event object.
 */
void event_button_files_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (files_screen_delay_timer) return;
    files_screen_delay_timer = lv_timer_create(
        files_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    files_screen_delay_timer_cb(NULL);
  }
}

/**
 * Handles the event triggered when a directory is clicked.
 *
 * @param e The event object containing the event data.
 */
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

/**
 * Handles the event triggered when a file is clicked.
 *
 * @param e The event object containing the event data.
 */
void event_file_handler(lv_event_t *e) {
  ESP3DFileDescriptor *file = (ESP3DFileDescriptor *)lv_event_get_user_data(e);

  esp3d_log("file Clicked: %s", file->name.c_str());

  std::string file_path_to_play = ESP3D_SD_FS_HEADER;
  file_path_to_play += files_path;
  if (esp3d_string::endsWith(file_path_to_play.c_str(), "/") == false) {
    file_path_to_play += "/";
  }
  file_path_to_play += file->name;
  file_path_to_play =
      esp3d_string::str_replace(file_path_to_play.c_str(), "//", "/");
  esp3d_log("file path : %s", file_path_to_play.c_str());
  if (gcodeHostService.addStream(file_path_to_play.c_str(),
                                 ESP3DAuthenticationLevel::admin, false)) {
    esp3d_log("Stream: %s added as main File", file_path_to_play.c_str());
  } else {
    esp3d_log_e("Failed to add stream");
  }
  // Should we force the main screen to update the file name  and status ?
  //
  event_button_files_back_handler(e);
}

void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  msg = NULL;
  // Screen creation
  esp3d_log("Files screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_new_screen)) {
    esp3d_log_e("Failed to create new screen");
    return;
  }
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  if (lv_obj_is_valid(ui_current_screen)) {
    lv_obj_del(ui_current_screen);
  }

  // button back
  lv_obj_t *btnback = backButton::create(ui_new_screen);
  if (!lv_obj_is_valid(btnback)) {
    esp3d_log_e("Failed to create back button");
    return;
  }
  lv_obj_add_event_cb(btnback, event_button_files_back_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_update_layout(btnback);

  // button refresh
  refresh_button = symbolButton::create(ui_new_screen, LV_SYMBOL_REFRESH,
                                        ESP3D_SYMBOL_BUTTON_WIDTH,
                                        lv_obj_get_height(btnback));
  if (!lv_obj_is_valid(refresh_button)) {
    esp3d_log_e("Failed to create refresh button");
    return;
  }
  lv_obj_align(refresh_button, LV_ALIGN_BOTTOM_MID, 0,
               -ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_add_event_cb(refresh_button, event_button_files_refresh_handler,
                      LV_EVENT_CLICKED, NULL);
  // label path
  lv_obj_t *labelpath = lv_label_create(ui_new_screen);
  if (!lv_obj_is_valid(labelpath)) {
    esp3d_log_e("Failed to create label");
    return;
  }
  lv_label_set_text(labelpath, files_path.c_str());
  lv_label_set_long_mode(labelpath, LV_LABEL_LONG_SCROLL_CIRCULAR);
  ESP3DStyle::apply(labelpath, ESP3DStyleType::bg_label);
  lv_obj_set_pos(labelpath, ESP3D_STATUS_BAR_V_PAD, ESP3D_STATUS_BAR_V_PAD);
  lv_obj_set_width(labelpath, LV_HOR_RES - (2 * ESP3D_STATUS_BAR_V_PAD));
  lv_obj_set_style_pad_left(labelpath, ESP3D_BUTTON_PRESSED_OUTLINE,
                            LV_PART_MAIN);
  lv_obj_set_style_pad_right(labelpath, ESP3D_BUTTON_PRESSED_OUTLINE,
                             LV_PART_MAIN);
  lv_label_set_long_mode(labelpath, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_update_layout(labelpath);

  // list control
  ui_files_list_ctl = lv_list_create(ui_new_screen);
  lv_obj_clear_flag(ui_files_list_ctl, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_pos(ui_files_list_ctl, 0,
                 lv_obj_get_height(labelpath) + (2 * ESP3D_STATUS_BAR_V_PAD));
  lv_obj_set_size(
      ui_files_list_ctl, LV_HOR_RES,
      LV_VER_RES -
          ((1.5 * ESP3D_BUTTON_PRESSED_OUTLINE) + lv_obj_get_height(btnback) +
           lv_obj_get_height(labelpath) + (2 * ESP3D_STATUS_BAR_V_PAD)));
  lv_obj_set_style_pad_left(ui_files_list_ctl, ESP3D_LIST_CONTAINER_LR_PAD,
                            LV_PART_MAIN);
  lv_obj_set_style_pad_right(ui_files_list_ctl, ESP3D_LIST_CONTAINER_LR_PAD,
                             LV_PART_MAIN);

  if (files_path != "/") {
    std::string tmplabel = ".." LV_SYMBOL_NEW_LINE;
    lv_obj_t *line1 = listLine::create(ui_files_list_ctl);
    if (!lv_obj_is_valid(line1)) {
      esp3d_log_e("Failed to create line");
      return;
    }
    lv_obj_t *btn_prev = listLine::add_button(tmplabel.c_str(), line1);
    lv_obj_add_event_cb(btn_prev, event_button_files_up_handler,
                        LV_EVENT_CLICKED, NULL);
  }
  // populate if not done
  if (first_fill_needed) {
    if (files_list.size() == 0) {
      event_button_files_refresh_handler(nullptr);
      lv_obj_add_flag(ui_files_list_ctl, LV_OBJ_FLAG_HIDDEN);
    }
    first_fill_needed = false;
  } else if (files_has_sd) {
    // dir first
    for (auto &file : files_list) {
      if (file.size == "-1") {
        lv_obj_t *line_container = listLine::create(ui_files_list_ctl);
        if (!lv_obj_is_valid(line_container)) {
          esp3d_log_e("Failed to create line");
          return;
        }
        listLine::add_label(LV_SYMBOL_FOLDER, line_container, false);
        listLine::add_label(file.name.c_str(), line_container, true);
        lv_obj_t *btn = listLine::add_button(LV_SYMBOL_SEARCH, line_container);
        lv_obj_add_event_cb(btn, event_directory_handler, LV_EVENT_CLICKED,
                            &file);
      }
    }

    for (auto &file : files_list) {
      if (file.size != "-1") {
        lv_obj_t *line_container = listLine::create(ui_files_list_ctl);
        if (!lv_obj_is_valid(line_container)) {
          esp3d_log_e("Failed to create line");
          return;
        }
        listLine::add_label(LV_SYMBOL_FILE, line_container, false);
        listLine::add_label(file.name.c_str(), line_container, true);
        listLine::add_label(file.size.c_str(), line_container, false);
        lv_obj_t *btn = listLine::add_button(LV_SYMBOL_PLAY, line_container);
        lv_obj_add_event_cb(btn, event_file_handler, LV_EVENT_CLICKED, &file);
      }
    }
  } else {
    if (lv_obj_is_valid(ui_files_list_ctl)) {
      lv_obj_del(ui_files_list_ctl);
    }
    ui_files_list_ctl = NULL;
    msg = lv_label_create(ui_new_screen);
    if (!lv_obj_is_valid(msg)) {
      esp3d_log_e("Failed to create label");
      return;
    }
    lv_obj_del(labelpath);
    lv_label_set_text(
        msg, esp3dTranslationService.translate(ESP3DLabel::no_sd_card));
    lv_obj_center(msg);
  }

  esp3dTftui.set_current_screen(ESP3DScreenType::files);
}

}  // namespace filesScreen

#endif  // ESP3D_SD_CARD_FEATURE