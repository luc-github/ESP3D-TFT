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

#include "spinner_component.h"

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace spinnerScreen {
lv_obj_t* spinnerObj = NULL;
size_t spinner_index = 0;
lv_obj_t* lblextra = NULL;
lv_obj_t* screen_displaying_spinner = NULL;

void update_spinner(const char* msg) {
  esp3d_log("Update spinner %s", msg);
  if (spinnerObj == NULL) {
    esp3d_log_w("Spinner not displayed");
    return;
  }
  if (msg != nullptr) {
    if (lblextra != NULL) {
      lv_label_set_text(lblextra, msg);
    }
  }
}

void show_spinner(const char* msg, lv_obj_t* backtbn) {
  // to avoid multiple call to show spinner
  esp3d_log("Show spinner");
  if (spinnerObj != NULL) {
    esp3d_log_w("Spinner already displayed");
    return;
  }
  spinner_index++;
  esp3d_log("Spinner index is %d", spinner_index);
  screen_displaying_spinner = lv_scr_act();
  spinnerObj = lv_obj_create(screen_displaying_spinner);
  apply_style(spinnerObj, ESP3DStyleType::spinner_screen);
  lv_obj_move_foreground(spinnerObj);
  lv_obj_set_size(spinnerObj, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(spinnerObj, 0, 0);
  lv_obj_t* spinner = lv_spinner_create(spinnerObj, 1000, 60);
  lv_obj_set_size(spinner, CURRENT_SPINNER_SIZE, CURRENT_SPINNER_SIZE);
  lv_obj_center(spinner);

  std::string text = esp3dTranslationService.translate(ESP3DLabel::please_wait);
  lv_obj_t* lbl = lv_label_create(spinnerObj);
  apply_style(lbl, ESP3DStyleType::spinner_text);
  lv_label_set_text(lbl, text.c_str());
  lv_obj_align_to(lbl, spinner, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  CURRENT_BUTTON_PRESSED_OUTLINE);
  lblextra = NULL;
  if (msg != nullptr) {
    lblextra = lv_label_create(spinnerObj);
    apply_style(lblextra, ESP3DStyleType::spinner_text);
    lv_label_set_text(lblextra, msg);
    lv_obj_align_to(lblextra, lbl, LV_ALIGN_OUT_BOTTOM_MID, 0,
                    CURRENT_BUTTON_PRESSED_OUTLINE);
  }
  if (backtbn != nullptr) {
    lv_obj_move_foreground(backtbn);
  }
}

void hide_spinner() {
  esp3d_log("Hide spinner, index is %d", spinner_index);
  if (spinnerObj != NULL) {
    lv_obj_add_flag(spinnerObj, LV_OBJ_FLAG_HIDDEN);
    if (screen_displaying_spinner != lv_scr_act()) {
      esp3d_log_e("Spinner not displayed on current screen");
      // so screen may be already deleted ?
    } else {
      esp3d_log("Spinner displayed on current screen");
      lv_obj_del(spinnerObj);
    }
    spinnerObj = NULL;
    screen_displaying_spinner = NULL;
    spinner_index--;
    esp3d_log("Spinner index is %d", spinner_index);
  } else {
    esp3d_log_w("Spinner already hidden");
  }
}

}  // namespace spinnerScreen