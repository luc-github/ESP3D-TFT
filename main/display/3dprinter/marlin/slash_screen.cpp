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
#include "esp3d_tft_ui.h"
#include "lvgl.h"
#include "version.h"


LV_IMG_DECLARE(logo_esp3d_tft);

/**********************
 *  STATIC PROTOTYPES
 **********************/
lv_timer_t *boot_timer = NULL;
lv_style_t style_version_text;

void main_screen();
void splash_screen();

void splash_in_timer_cb(lv_timer_t *timer) {
  // If timer is not null, delete it to avoid multiple call
  if (boot_timer) {
    lv_timer_del(boot_timer);
    boot_timer = NULL;
  }
  // Call splash screen
  splash_screen();
}

void main_screen_timer_cb(lv_timer_t *timer) {
  // If timer is not null, delete it to avoid multiple call
  if (boot_timer) {
    lv_timer_del(boot_timer);
    boot_timer = NULL;
  }
  // Call main screen
  main_screen();
}

void boot_screen() {
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN);
  boot_timer = lv_timer_create(splash_in_timer_cb, 10, NULL);
}

void splash_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::splash);
  // Get active screen
  lv_obj_t *ui_Screen = lv_scr_act();
  // Create logo object
  lv_obj_t *logo = lv_img_create(ui_Screen);
  // Set logo image
  lv_img_set_src(logo, &logo_esp3d_tft);
  // Create version text object
  lv_obj_t *label = lv_label_create(ui_Screen);
  // Set version text
  lv_label_set_text(label, "V" ESP3D_TFT_VERSION);
  // Create style for version text
  lv_style_init(&style_version_text);
  lv_style_set_text_opa(&style_version_text, LV_OPA_COVER);
  lv_style_set_text_color(&style_version_text, lv_color_hex(0xFFFFFF));
  // Apply style to version text
  lv_obj_add_style(label, &style_version_text, LV_PART_MAIN);
  // align object in screen
  lv_obj_center(logo);
  lv_obj_align(label, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
  // Set timer to switch to main screen
  boot_timer = lv_timer_create(main_screen_timer_cb, 2000, NULL);
}
