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

#include "screens/splash_screen.h"

#include <lvgl.h>

#include "esp3d_log.h"
#include "esp3d_lvgl.h"
#include "esp3d_styles.h"
#include "esp3d_styles_splash_screen.h"
#include "esp3d_tft_ui.h"
#include "esp3d_version.h"
#include "screens/main_screen.h"

// Images are stored in the flash memory
// the logo is an rle image
extern "C" lv_img_dsc_t *get_splash_logo();
extern "C" void release_splash_logo(lv_img_dsc_t *splash_logo);
// no rle image but normal one for the target logo
LV_IMG_DECLARE(target_fw_logo);

// namespace for the splash screen
namespace splashScreen {
// Static variables
lv_timer_t *boot_timer = NULL;
lv_img_dsc_t *splash_logo = NULL;

// Static functions
void display();

/** @brief Callback function for the splash screen timer.
 *
 * This function is called when the splash screen timer expires. It deletes the
 * timer to avoid multiple calls and then calls the display function.
 *
 * @param timer The timer object that triggered the callback.
 */
void splash_in_timer_cb(lv_timer_t *timer) {
  // If timer is not null, delete it to avoid multiple call
  if (boot_timer && lv_timer_is_valid(boot_timer)) {
    lv_timer_del(boot_timer);
  }
  boot_timer = NULL;
  // Call display function
  display();
}

/**
 * @brief Function to clean up resources and exit the splash screen.
 *
 * This function deletes the boot timer if it is not null and releases the
 * splash logo if it is not null.
 */
void leave() {
  // If timer is not null, delete it to avoid multiple call
  if (boot_timer && lv_timer_is_valid(boot_timer)) {
    lv_timer_del(boot_timer);
  }
  boot_timer = NULL;
  // If splash_logo is not null, release it
  if (splash_logo != NULL) {
    release_splash_logo(splash_logo);
    splash_logo = NULL;
  }
}

/**
 * Callback function for the main screen timer.
 * This function is called when the timer expires.
 * It calls the leave function and then the main screen function.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void main_screen_timer_cb(lv_timer_t *timer) {
  // Call leave function
  leave();
  // Call main screen
  mainScreen::create();
}

/**
 * @brief Creates the splash screen.
 *
 * This function applies the main background style to the active screen and
 * creates a timer for the splash screen animation. If the timer creation fails,
 * an error message is logged and the function returns.
 */
void create() {
  ESP3DStyle::apply(lv_scr_act(), ESP3DStyleType::main_bg);
  boot_timer = lv_timer_create(splash_in_timer_cb, 10, NULL);
  if (boot_timer == NULL) {
    esp3d_log_e("Failed to create timer");
    return;
  }
}

/**
 * @brief Displays the splash screen.
 *
 * This function sets up and displays the splash screen on the UI. It creates
 * and positions the splash logo, target logo, and version text objects. It also
 * sets a timer to switch to the main screen after a certain duration.
 *
 * @note This function assumes that the UI has already been initialized.
 */
void display() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Get active screen
  lv_obj_t *ui_Screen = lv_scr_act();
  // get splash logo
  splash_logo = get_splash_logo();
  if (splash_logo != NULL) {
    // Create logo object
    lv_obj_t *logo = lv_img_create(ui_Screen);
    if (!lv_obj_is_valid(logo)) {
      esp3d_log_e("Invalid logo object");
      return;
    }
    // Set logo image
    lv_img_set_src(logo, splash_logo);
    // align object in screen
    lv_obj_center(logo);
  }

  // Set target Logo
  lv_obj_t *logo_target = lv_img_create(ui_Screen);
  if (!lv_obj_is_valid(logo_target)) {
    esp3d_log_e("Invalid target logo object");
    return;
  }
  lv_img_set_src(logo_target, &target_fw_logo);
  lv_obj_align(logo_target, LV_ALIGN_BOTTOM_LEFT, FW_LOGO_X, FW_LOGO_Y);

  // Create version text object
  lv_obj_t *label = lv_label_create(ui_Screen);
  if (!lv_obj_is_valid(label)) {
    esp3d_log_e("Invalid label object");
    return;
  }
  // Set version text
  lv_label_set_text(label, "V" ESP3D_TFT_VERSION);
  ESP3DStyle::apply(label, ESP3DStyleType::bg_label);
  lv_obj_align(label, LV_ALIGN_BOTTOM_RIGHT, VERSION_X, VERSION_Y);

  // Set timer to switch to main screen
  boot_timer = lv_timer_create(main_screen_timer_cb, 2000, NULL);
  if (boot_timer == NULL) {
    esp3d_log_e("Failed to create timer");
    return;
  }
  esp3dTftui.set_current_screen(ESP3DScreenType::splash);
}

}  // namespace splashScreen