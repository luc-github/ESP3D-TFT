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

#include "splash_screen.h"

#include <lvgl.h>

#include "esp3d_log.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "esp3d_version.h"
#include "screens/main_screen.h"

extern "C" lv_img_dsc_t *get_splash_logo();
extern "C" void release_splash_logo(lv_img_dsc_t *splash_logo);
LV_IMG_DECLARE(target_fw_logo);

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace splashScreen {
lv_timer_t *boot_timer = NULL;
lv_img_dsc_t *splash_logo = NULL;

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
  if (splash_logo != NULL) {
    release_splash_logo(splash_logo);
  }
  // Call main screen
  mainScreen::main_screen();
}

void boot_screen() {
  apply_style(lv_scr_act(), ESP3DStyleType::main_bg);
  boot_timer = lv_timer_create(splash_in_timer_cb, 10, NULL);
}

void splash_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Get active screen
  lv_obj_t *ui_Screen = lv_scr_act();

  splash_logo = get_splash_logo();
  if (splash_logo != NULL) {
    // Create logo object
    lv_obj_t *logo = lv_img_create(ui_Screen);
    // Set logo image
    lv_img_set_src(logo, splash_logo);
    // align object in screen
    lv_obj_center(logo);
  }

  //Marlin Logo
  lv_obj_t *logo_marlin = lv_img_create(ui_Screen);
  lv_img_set_src(logo_marlin, &target_fw_logo);
  lv_obj_align(logo_marlin, LV_ALIGN_BOTTOM_LEFT, FW_LOGO_X, FW_LOGO_Y);

  // Create version text object
  lv_obj_t *label = lv_label_create(ui_Screen);
  // Set version text
  lv_label_set_text(label, "V" ESP3D_TFT_VERSION);
  apply_style(label, ESP3DStyleType::bg_label);
  lv_obj_align(label, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

  // Set timer to switch to main screen
  boot_timer = lv_timer_create(main_screen_timer_cb, 2000, NULL);
  esp3dTftui.set_current_screen(ESP3DScreenType::splash);
}

}  // namespace splashScreen