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

#include "screens/basic_screen.h"

#include <lvgl.h>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_lvgl.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  Namespace
 **********************/
namespace basicScreen {
// Static prototypes
void init();
void display();
void leave();

/**********************
 *   STATIC VARIABLES
 **********************/
bool intialization_done = false;
lv_obj_t *ui_screen = NULL;

/**********************
 *   LOCAL FUNCTIONS
 **********************/
/**
 * Initializes the screen.
 * This function performs the initialization of the screen.
 * It checks if the initialization has already been done and returns if so.
 * Otherwise, it performs the necessary initialization steps.
 *
 * @return void
 */
void init() {
  if (intialization_done) {
    return;
  }
  // Intialisation to be done once
  // TODO: Add your code here
  intialization_done = true;
}

/**
 * @brief Displays the  screen.
 *
 * This function sets the current screen to ESP3DScreenType::empty and so enable
 * the screen.
 */
void display() {
  esp3dTftui.set_current_screen(ESP3DScreenType::empty);
  // TODO: Add your code here
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
 * @brief Creates a new screen and displays it.
 *
 * This function sets the current screen type to none, creates a new screen, and
 * displays it on the display. It also deletes the old screen if it exists. The
 * function then initializes the screen and calls the display function to show
 * the screen.
 *
 * @note Make sure to add your code in the designated TODO section.
 */
void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);

  // Do initialization if not done
  init();
  // Create screen (always needed)
  ui_screen = lv_obj_create(NULL);
  if (!lv_obj_is_valid(ui_screen)) {
    esp3d_log_e("Failed to create screen");
    return;
  }
  // Display new screen now and delete old one to save memory
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_screen);
  // Free old screen
  if (lv_obj_is_valid(ui_current_screen)) {
    lv_obj_del(ui_current_screen);
  }
  // TODO: Add your code here

  // Display screen
  display();
}

/**
 * @brief Leaves the current screen and performs necessary actions.
 *
 * This function is called when the user wants to leave the current screen and
 * navigate to another screen. Any necessary actions or cleanup can be performed
 * in this function.
 *
 * @note This function is currently empty and needs to be implemented.
 */
void leave() {
  // TODO: Add your code here
}
}  // namespace basicScreen