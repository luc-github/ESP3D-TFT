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

#include "screens/informations_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/main_container_component.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "esp3d_version.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "filesystem/esp3d_flash.h"
#include "rom/ets_sys.h"
#include "screens/menu_screen.h"
#include "sdkconfig.h"
#include "spi_flash_mmap.h"
#include "translations/esp3d_translation_service.h"

#if CONFIG_SPIRAM
#include "esp_psram.h"
#endif  // CONFIG_SPIRAM

#if ESP3D_UPDATE_FEATURE
#include "update/esp3d_update_service.h"
#endif  // ESP3D_UPDATE_FEATURE

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace informationsScreen {
lv_timer_t *informations_screen_delay_timer = NULL;

void informations_screen_delay_timer_cb(lv_timer_t *timer) {
  if (informations_screen_delay_timer) {
    lv_timer_del(informations_screen_delay_timer);
    informations_screen_delay_timer = NULL;
  }
  menuScreen::create();
}

void event_button_informations_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (informations_screen_delay_timer) return;
    informations_screen_delay_timer = lv_timer_create(
        informations_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else
    informations_screen_delay_timer_cb(NULL);
}

void addInformationToList(lv_obj_t *list, ESP3DLabel label, const char *info) {
  std::string infoStr = esp3dTranslationService.translate(label);
  infoStr += ": ";
  infoStr += info;
  lv_list_add_btn(list, "", infoStr.c_str());
}

void addInformationToList(lv_obj_t *list, ESP3DLabel label, ESP3DLabel info) {
  std::string infoStr = esp3dTranslationService.translate(label);
  infoStr += ": ";
  infoStr += esp3dTranslationService.translate(info);
  lv_list_add_btn(list, "", infoStr.c_str());
}

void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Settings screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  ESP3DStyle::apply(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_del(ui_current_screen);

  lv_obj_t *btnback = backButton::create(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_informations_back_handler,
                      LV_EVENT_CLICKED, NULL);
  lv_obj_update_layout(btnback);
  lv_obj_set_style_flex_flow(ui_new_screen, LV_FLEX_FLOW_ROW,
                             LV_FLEX_ALIGN_SPACE_EVENLY);
  lv_obj_t *ui_info_list_ctl = lv_list_create(ui_new_screen);
  ESP3DStyle::apply(ui_info_list_ctl, ESP3DStyleType::status_list);

  lv_obj_update_layout(ui_new_screen);
  lv_obj_set_pos(ui_info_list_ctl, ESP3D_BUTTON_PRESSED_OUTLINE,
                 ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_set_size(
      ui_info_list_ctl, LV_HOR_RES - ESP3D_BUTTON_PRESSED_OUTLINE * 2,
      lv_obj_get_height(ui_new_screen) - (ESP3D_BUTTON_PRESSED_OUTLINE * 3) -
          lv_obj_get_height(btnback));
  lv_obj_set_style_radius(ui_info_list_ctl, ESP3D_CONTAINER_RADIUS, 0);
  lv_obj_set_style_pad_left(ui_info_list_ctl, ESP3D_LIST_CONTAINER_LR_PAD,
                            LV_PART_MAIN);
  lv_obj_set_style_pad_right(ui_info_list_ctl, ESP3D_LIST_CONTAINER_LR_PAD,
                             LV_PART_MAIN);

  addInformationToList(ui_info_list_ctl, ESP3DLabel::screen, TFT_TARGET);
  addInformationToList(ui_info_list_ctl, ESP3DLabel::version,
                       ESP3D_TFT_VERSION);
  addInformationToList(ui_info_list_ctl, ESP3DLabel::architecture,
                       CONFIG_IDF_TARGET);
  addInformationToList(ui_info_list_ctl, ESP3DLabel::sdk_version, IDF_VER);
  std::string tmpstr = std::to_string(ets_get_cpu_frequency()) + "MHz";
  addInformationToList(ui_info_list_ctl, ESP3DLabel::cpu_freq, tmpstr.c_str());

  // Free memory
  tmpstr = esp3d_string::formatBytes(esp_get_free_heap_size());
  addInformationToList(ui_info_list_ctl, ESP3DLabel::free_heap, tmpstr.c_str());

#if CONFIG_SPIRAM
  tmpstr = esp3d_string::formatBytes(esp_psram_get_size());
  addInformationToList(ui_info_list_ctl, ESP3DLabel::total_psram,
                       tmpstr.c_str());
#endif  // CONFIG_SPIRAM

  // Flash size
  uint32_t flash_size;
  if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
    esp3d_log_e("Get flash size failed");
    flash_size = 0;
  }
  tmpstr = esp3d_string::formatBytes(flash_size);
  addInformationToList(ui_info_list_ctl, ESP3DLabel::flash_size,
                       tmpstr.c_str());

#if ESP3D_UPDATE_FEATURE
  // Update max
  tmpstr = esp3d_string::formatBytes(esp3dUpdateService.maxUpdateSize());
  addInformationToList(ui_info_list_ctl, ESP3DLabel::size_for_update,
                       tmpstr.c_str());
#if ESP3D_SD_CARD_FEATURE
  // SD updater
  ESP3DState statesetting = (ESP3DState)esp3dTftsettings.readByte(
      ESP3DSettingIndex::esp3d_check_update_on_sd);
  if (statesetting == ESP3DState::off) {
    tmpstr = esp3dTranslationService.translate(ESP3DLabel::off);
  } else {
    tmpstr = esp3dTranslationService.translate(ESP3DLabel::on);
  }
  addInformationToList(ui_info_list_ctl, ESP3DLabel::sd_updater,
                       tmpstr.c_str());
#endif  // ESP3D_SD_CARD_FEATURE
#else
  addInformationToList(ui_info_list_ctl, ESP3DLabel::sd_updater,
                       ESP3DLabel::off);
#endif  // ESP3D_UPDATE_FEATURE

  // Flash type
  addInformationToList(ui_info_list_ctl, ESP3DLabel::flash_type,
                       flashFs.getFileSystemName());

  // Target fw
  addInformationToList(ui_info_list_ctl, ESP3DLabel::target_firmware, "Grbl");

  esp3dTftui.set_current_screen(ESP3DScreenType::informations);
}
}  // namespace informationsScreen