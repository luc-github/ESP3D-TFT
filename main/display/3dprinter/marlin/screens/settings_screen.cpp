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

#include "settings_screen.h"

#include <lvgl.h>

#include "components/back_button_component.h"
#include "components/choice_editor_component.h"
#include "components/list_line_component.h"
#include "components/main_container_component.h"
#include "components/message_box_component.h"
#include "components/spinner_component.h"
#include "components/text_editor_component.h"
#include "esp3d_client_types.h"
#include "esp3d_json_settings.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "main_screen.h"
#include "manual_leveling_screen.h"
#include "menu_screen.h"
#include "rendering/esp3d_rendering_client.h"
#include "tasks_def.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace settingsScreen {
#define STACKDEPTH 4096
#define TASKPRIORITY UI_TASK_PRIORITY - 1
#define TASKCORE UI_TASK_CORE

lv_timer_t *settings_screen_delay_timer = NULL;
lv_timer_t *settings_screen_apply_timer = NULL;
lv_obj_t *ui_settings_list_ctl = NULL;
lv_obj_t *hostname_label = NULL;
lv_obj_t *extensions_label = NULL;
lv_obj_t *show_fan_controls_label = NULL;
lv_obj_t *output_client_label = NULL;
lv_obj_t *serial_baud_rate_label = NULL;
lv_obj_t *usb_serial_baud_rate_label = NULL;
lv_obj_t *jog_type_label = NULL;
lv_obj_t *polling_label = NULL;
lv_obj_t *auto_leveling_label = NULL;
lv_obj_t *bed_width_label = NULL;
lv_obj_t *bed_depth_label = NULL;
lv_obj_t *inverted_x_label = NULL;
lv_obj_t *inverted_y_label = NULL;

struct ESP3DSettingsData {
  ESP3DSettingIndex index;
  std::string value;
  lv_obj_t *label;
  std::list<std::string> choices;
  std::string entry = "";
};

void settings_screen_delay_timer_cb(lv_timer_t *timer) {
  if (settings_screen_delay_timer) {
    lv_timer_del(settings_screen_delay_timer);
    settings_screen_delay_timer = NULL;
  }
  menuScreen::menu_screen();
}

// refresh_settings_list_cb
void refresh_settings_list_cb(lv_timer_t *timer) {
  bool refresh = true;
  if (timer->user_data) {
    refresh = *(bool *)((timer->user_data));
  }
  if (settings_screen_apply_timer) {
    lv_timer_del(settings_screen_apply_timer);
    settings_screen_apply_timer = NULL;
  }
  spinnerScreen::hide_spinner();
  if (refresh) settings_screen();
}

// bgLoadJSONSettingsTask
static void bgLoadJSONSettingsTask(void *pvParameter) {
  (void)pvParameter;
  vTaskDelay(pdMS_TO_TICKS(100));
  std::string value =
      esp3dTftJsonSettings.readString("settings", "filesfilter");
  if (extensions_label) {
    lv_label_set_text(extensions_label, value.c_str());
  }
  value = esp3dTftJsonSettings.readString("settings", "showfanctrls");
  if (show_fan_controls_label) {
    if (value == "true") {
      value = esp3dTranslationService.translate(ESP3DLabel::enabled);
    } else {
      value = esp3dTranslationService.translate(ESP3DLabel::disabled);
    }
    lv_label_set_text(show_fan_controls_label, value.c_str());
  }
  static bool refresh = false;
  if (!settings_screen_apply_timer) {
    settings_screen_apply_timer =
        lv_timer_create(refresh_settings_list_cb, 100, &refresh);
  }
  vTaskDelete(NULL);
}

// bgSaveJSONSettingsTask
static void bgSaveJSONSettingsTask(void *pvParameter) {
  vTaskDelay(pdMS_TO_TICKS(100));
  ESP3DSettingsData *data = (ESP3DSettingsData *)pvParameter;
  esp3d_log("Got value %s in task", data->value.c_str());
  // do the change

  esp3d_log("Value %s is valid", data->value.c_str());
  if (esp3dTftJsonSettings.writeString("settings", data->entry.c_str(),
                                       data->value.c_str())) {
    switch (data->index) {
      case ESP3DSettingIndex::esp3d_extensions:
        if (data->label) {
          lv_label_set_text(data->label, data->value.c_str());
        }
        break;
      case ESP3DSettingIndex::esp3d_show_fan_controls:
        if (strcmp(data->value.c_str(), "true") == 0) {
          mainScreen::update_show_fan_controls(true);
          if (data->label) {
            lv_label_set_text(data->label, esp3dTranslationService.translate(
                                               ESP3DLabel::enabled));
          }
        } else {
          mainScreen::update_show_fan_controls(false);
          if (data->label) {
            lv_label_set_text(data->label, esp3dTranslationService.translate(
                                               ESP3DLabel::disabled));
          }
        }
        break;
      default:
        break;
    }

  } else {
    esp3d_log_e("Failed to save %s", data->entry.c_str());
    std::string text =
        esp3dTranslationService.translate(ESP3DLabel::error_applying_setting);
    msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
  }

  if (!settings_screen_apply_timer) {
    settings_screen_apply_timer =
        lv_timer_create(refresh_settings_list_cb, 100, NULL);
  }
  vTaskDelete(NULL);
}

void CreateSaveJSONSettingTask(ESP3DSettingsData *settingData) {
  spinnerScreen::show_spinner();
  TaskHandle_t xHandle = NULL;

  BaseType_t res = xTaskCreatePinnedToCore(
      bgSaveJSONSettingsTask, "savesettingsTask", STACKDEPTH,
      (void *)(settingData), TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Settings Task");
  } else {
    esp3d_log_e("Settings Task creation failed");
  }
}

// setting_edit_done_cb
void setting_edit_done_cb(const char *str, void *data) {
  if (!data) {
    esp3d_log_e("No data provided");
    return;
  }
  esp3d_log("Saving setting to: %s\n", str);
  ESP3DSettingsData *settingData = (ESP3DSettingsData *)data;
  if (settingData->value != str) {
    // get value type
    const ESP3DSettingDescription *settingPtr = NULL;
    if (settingData->entry != "") {
      esp3dTftsettings.getSettingPtr(settingData->index);
    }
    bool isValid = false;
    bool success_saving = false;
    uint8_t val_byte = 0;
    uint32_t val_uint32 = 0;
    std::string val_string = str;
    if (settingPtr != NULL) {
      switch (settingPtr->type) {
        case ESP3DSettingType::byte_t:
          switch (settingData->index) {
            case ESP3DSettingIndex::esp3d_output_client:
              if (strcmp(str, esp3dTranslationService.translate(
                                  ESP3DLabel::usb)) == 0) {
                val_byte = (uint8_t)ESP3DClientType::usb_serial;
              } else if (strcmp(str, esp3dTranslationService.translate(
                                         ESP3DLabel::serial)) == 0) {
                val_byte = (uint8_t)ESP3DClientType::serial;
              }
              break;
            case ESP3DSettingIndex::esp3d_jog_type:
              if (strcmp(str, esp3dTranslationService.translate(
                                  ESP3DLabel::absolute)) == 0) {
                val_byte = 1;
              } else if (strcmp(str, esp3dTranslationService.translate(
                                         ESP3DLabel::relative)) == 0) {
                val_byte = 0;
              } else {
                esp3d_log_e("Unknown jog type %s", str);
                return;
              }
              break;
            case ESP3DSettingIndex::esp3d_auto_level_on:
            case ESP3DSettingIndex::esp3d_inverved_x:
            case ESP3DSettingIndex::esp3d_inverved_y:
            case ESP3DSettingIndex::esp3d_polling_on:
              if (strcmp(str, esp3dTranslationService.translate(
                                  ESP3DLabel::enabled)) == 0) {
                val_byte = 1;
              } else if (strcmp(str, esp3dTranslationService.translate(
                                         ESP3DLabel::disabled)) == 0) {
                val_byte = 0;
              } else {
                esp3d_log_e("Unknown state %s", str);
                return;
              }
              break;
            default:
              esp3d_log_e("Unknown setting index %d",
                          (uint16_t)settingData->index);
              return;
          }

          isValid =
              esp3dTftsettings.isValidByteSetting(val_byte, settingData->index);
          if (isValid) {
            success_saving =
                esp3dTftsettings.writeByte(settingData->index, val_byte);
          }
          break;
        case ESP3DSettingType::integer_t:
          switch (settingData->index) {
            case ESP3DSettingIndex::esp3d_baud_rate:
            case ESP3DSettingIndex::esp3d_usb_serial_baud_rate:
              val_uint32 = (uint32_t)atoi(str);
              break;
            default:
              esp3d_log_e("Unknown setting index %d",
                          (uint16_t)settingData->index);
              return;
          }
          isValid = esp3dTftsettings.isValidIntegerSetting(val_uint32,
                                                           settingData->index);
          if (isValid) {
            success_saving =
                esp3dTftsettings.writeUint32(settingData->index, val_uint32);
          }
          break;
        case ESP3DSettingType::float_t:
        case ESP3DSettingType::string_t:
          isValid =
              esp3dTftsettings.isValidStringSetting(str, settingData->index);
          if (isValid) {
            switch (settingData->index) {
              case ESP3DSettingIndex::esp3d_hostname:
                // use string as it is
                break;
              case ESP3DSettingIndex::esp3d_bed_width:
              case ESP3DSettingIndex::esp3d_bed_depth:
                val_string = esp3d_string::set_precision(str, 2);
                break;
              default:
                esp3d_log_e("Unknown setting index %d",
                            (uint16_t)settingData->index);
                return;
            }

            success_saving = esp3dTftsettings.writeString(settingData->index,
                                                          val_string.c_str());
          }
          break;
        case ESP3DSettingType::ip:
        case ESP3DSettingType::mask:
        case ESP3DSettingType::bitsfield:
          esp3d_log_e("This type %d, is not yet supported",
                      (uint8_t)settingPtr->type);
          return;
        default:
          esp3d_log_e("Unknown type %d", (uint8_t)settingPtr->type);
          return;
      }
    } else {
      switch (settingData->index) {
        case ESP3DSettingIndex::esp3d_extensions:
          val_string = str;
          break;
        case ESP3DSettingIndex::esp3d_show_fan_controls:
          if (strcmp(str, esp3dTranslationService.translate(
                              ESP3DLabel::enabled)) == 0) {
            val_string = "true";
          } else if (strcmp(str, esp3dTranslationService.translate(
                                     ESP3DLabel::disabled)) == 0) {
            val_string = "false";
          } else {
            esp3d_log_e("Unknown state %s", str);
            return;
          }
          break;
        default:
          esp3d_log_e("Unknown setting index %d", (uint16_t)settingData->index);
          return;
      }
      settingData->value = val_string;
      CreateSaveJSONSettingTask(settingData);
      return;
    }

    if (!isValid) {
      esp3d_log_e("Invalid value %s", str);
      std::string text =
          esp3dTranslationService.translate(ESP3DLabel::error_applying_setting);
      msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      return;
    }
    if (!success_saving) {
      esp3d_log_e("Failed to save setting %s", str);
      std::string text =
          esp3dTranslationService.translate(ESP3DLabel::error_applying_setting);
      msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      return;
    }
    if (settingData->label) {
      lv_label_set_text(settingData->label, val_string.c_str());
    }
    // now apply the setting if needed
    switch (settingData->index) {
      case ESP3DSettingIndex::esp3d_polling_on:
        renderingClient.setPolling(val_byte);
        break;
      case ESP3DSettingIndex::esp3d_inverved_x:
        manualLevelingScreen::update_invert_x(val_byte);
        break;
      case ESP3DSettingIndex::esp3d_inverved_y:
        manualLevelingScreen::update_invert_y(val_byte);
        break;
      case ESP3DSettingIndex::esp3d_auto_level_on:
        menuScreen::enable_auto_leveling(val_byte);
        break;
      case ESP3DSettingIndex::esp3d_bed_width:
        manualLevelingScreen::update_bed_width(
            strtod(val_string.c_str(), NULL));
        break;
      case ESP3DSettingIndex::esp3d_bed_depth:
        manualLevelingScreen::update_bed_depth(
            strtod(val_string.c_str(), NULL));
        break;
      default:
        break;
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// event_button_edit_setting
void event_button_edit_setting_cb(lv_event_t *e) {
  esp3d_log("Show component editor");
  static ESP3DSettingsData data;
  data.index = *((ESP3DSettingIndex *)(lv_event_get_user_data(e)));
  data.choices.clear();
  data.entry = "";
  data.label = NULL;
  std::string title;
  uint8_t list_size = 0;
  switch (data.index) {
    case ESP3DSettingIndex::esp3d_baud_rate:
      data.label = serial_baud_rate_label;
      title = esp3dTranslationService.translate(ESP3DLabel::serial_baud_rate);
      list_size = sizeof(SupportedBaudList) / sizeof(uint32_t);
      for (uint8_t i = 0; i < list_size; i++) {
        data.choices.push_back(std::to_string(SupportedBaudList[i]));
      }
      break;
    case ESP3DSettingIndex::esp3d_usb_serial_baud_rate:
      data.label = usb_serial_baud_rate_label;
      title = esp3dTranslationService.translate(ESP3DLabel::usb_baud_rate);
      list_size = sizeof(SupportedBaudList) / sizeof(uint32_t);
      for (uint8_t i = 0; i < list_size; i++) {
        data.choices.push_back(std::to_string(SupportedBaudList[i]));
      }
      break;
    case ESP3DSettingIndex::esp3d_output_client:
      data.label = output_client_label;
      title = esp3dTranslationService.translate(ESP3DLabel::output_client);
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::serial));  // serial
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::usb));  // usb
      break;
    case ESP3DSettingIndex::esp3d_jog_type:
      data.label = jog_type_label;
      title = esp3dTranslationService.translate(ESP3DLabel::jog_type);
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::relative));  // relative
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::absolute));  // absolute
      break;
    case ESP3DSettingIndex::esp3d_polling_on:
      data.label = polling_label;
      title = esp3dTranslationService.translate(ESP3DLabel::polling);
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::disabled));  // disabled
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::enabled));  // enabled
      break;
    case ESP3DSettingIndex::esp3d_inverved_x:
      data.label = inverted_x_label;
      title = esp3dTranslationService.translate(ESP3DLabel::invert_axis, "X");
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::disabled));  // disabled
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::enabled));  // enabled
      break;
    case ESP3DSettingIndex::esp3d_inverved_y:
      data.label = inverted_y_label;
      title = esp3dTranslationService.translate(ESP3DLabel::invert_axis, "Y");
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::disabled));  // disabled
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::enabled));  // enabled
      break;
    case ESP3DSettingIndex::esp3d_auto_level_on:
      data.label = auto_leveling_label;
      title = esp3dTranslationService.translate(ESP3DLabel::auto_leveling);
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::disabled));  // disabled
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::enabled));  // enabled
      break;
    case ESP3DSettingIndex::esp3d_hostname:
      data.label = hostname_label;
      title = esp3dTranslationService.translate(ESP3DLabel::hostname);
      break;
    case ESP3DSettingIndex::esp3d_bed_width:
      data.label = bed_width_label;
      title = esp3dTranslationService.translate(ESP3DLabel::bed_width);
      break;
    case ESP3DSettingIndex::esp3d_bed_depth:
      data.label = bed_depth_label;
      title = esp3dTranslationService.translate(ESP3DLabel::bed_depth);
      break;
    case ESP3DSettingIndex::esp3d_extensions:
      data.label = extensions_label;
      title = esp3dTranslationService.translate(ESP3DLabel::extensions);
      data.entry = "filesfilter";
      break;
    case ESP3DSettingIndex::esp3d_show_fan_controls:
      data.label = show_fan_controls_label;
      title = esp3dTranslationService.translate(ESP3DLabel::fan_controls);
      data.entry = "showfanctrls";
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::disabled));  // disabled
      data.choices.push_back(
          esp3dTranslationService.translate(ESP3DLabel::enabled));  // enabled
      break;
    default:
      esp3d_log_e("Unknown setting index %d", (uint16_t)data.index);
      return;
  }
  data.value = lv_label_get_text(data.label);
  if (data.choices.size() > 0) {
    choiceEditor::create_choice_editor(lv_scr_act(), data.value.c_str(),
                                       title.c_str(), data.choices,
                                       setting_edit_done_cb, (void *)(&data));
  } else {
    // it is json setting so no getSettingPtr
    if (data.entry != "") {
      switch (data.index) {
        case ESP3DSettingIndex::esp3d_extensions:
          textEditor::create_text_editor(lv_scr_act(), data.value.c_str(),
                                         setting_edit_done_cb, 0, NULL, false,
                                         (void *)(&data));
          break;
        default:
          esp3d_log_e("Unknown setting index %d", (uint16_t)data.index);
          return;
      }
    } else {
      const ESP3DSettingDescription *settingPtr =
          esp3dTftsettings.getSettingPtr(data.index);
      switch (data.index) {
        case ESP3DSettingIndex::esp3d_hostname:
          textEditor::create_text_editor(lv_scr_act(), data.value.c_str(),
                                         setting_edit_done_cb, settingPtr->size,
                                         NULL, false, (void *)(&data));
          break;
        case ESP3DSettingIndex::esp3d_bed_width:
        case ESP3DSettingIndex::esp3d_bed_depth:
          textEditor::create_text_editor(lv_scr_act(), data.value.c_str(),
                                         setting_edit_done_cb, 15,
                                         "0123456789.", true, (void *)(&data));
          break;
        default:
          esp3d_log_e("Unknown setting index %d", (uint16_t)data.index);
          return;
      }
    }
  }
}

// event_button_settings_back_handler
void event_button_settings_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (BUTTON_ANIMATION_DELAY) {
    if (settings_screen_delay_timer) return;
    settings_screen_delay_timer = lv_timer_create(
        settings_screen_delay_timer_cb, BUTTON_ANIMATION_DELAY, NULL);
  } else {
    settings_screen_delay_timer_cb(NULL);
  }
}

void settings_screen() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Settings screen creation");
  lv_obj_t *ui_new_screen = lv_obj_create(NULL);
  // Display new screen and delete old one
  lv_obj_t *ui_current_screen = lv_scr_act();
  lv_scr_load(ui_new_screen);
  lv_obj_del(ui_current_screen);
  apply_style(ui_new_screen, ESP3DStyleType::main_bg);
  lv_obj_t *btnback = backButton::create_back_button(ui_new_screen);
  lv_obj_add_event_cb(btnback, event_button_settings_back_handler,
                      LV_EVENT_CLICKED, NULL);

  ui_settings_list_ctl = lv_list_create(ui_new_screen);
  lv_obj_clear_flag(ui_settings_list_ctl, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_style_pad_left(ui_settings_list_ctl, LIST_CONTAINER_LR_PAD, LV_PART_MAIN);
  lv_obj_set_style_pad_right(ui_settings_list_ctl, LIST_CONTAINER_LR_PAD, LV_PART_MAIN);

  lv_obj_set_size(
      ui_settings_list_ctl, LV_HOR_RES - CURRENT_BUTTON_PRESSED_OUTLINE * 2,
      LV_VER_RES -
          ((3 * CURRENT_BUTTON_PRESSED_OUTLINE) + lv_obj_get_height(btnback)));

  lv_obj_set_pos(ui_settings_list_ctl, CURRENT_BUTTON_PRESSED_OUTLINE,
                 CURRENT_BUTTON_PRESSED_OUTLINE);
  lv_obj_t *line_container = NULL;
  std::string LabelStr = "";
  // Hostname
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::hostname);
  if (line_container) {
    std::string hostname;
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_hostname);
    if (settingPtr) {
      char out_str[(settingPtr->size) + 1] = {0};
      hostname = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_hostname,
                                             out_str, settingPtr->size);
    }
    hostname_label =
        listLine::add_label_to_line(hostname.c_str(), line_container, true);
    lv_obj_t *btnEdit =
        listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)(&(settingPtr->index)));
  }

  // Extensions
  static ESP3DSettingIndex extensions_setting_index =
      ESP3DSettingIndex::esp3d_extensions;
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::extensions);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    extensions_label = listLine::add_label_to_line("", line_container, true);
    lv_obj_t *btnEdit =
        listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)&(extensions_setting_index));
  }

#if ESP3D_USB_SERIAL_FEATURE
  // USB Serial
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::output_client);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_output_client);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_output_client);
      std::string value =
          val == (uint8_t)ESP3DClientType::serial
              ? esp3dTranslationService.translate(ESP3DLabel::serial)
          : val == (uint8_t)ESP3DClientType::usb_serial
              ? esp3dTranslationService.translate(ESP3DLabel::usb)
              : "???";
      output_client_label =
          listLine::add_label_to_line(value.c_str(), line_container, true);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }
#endif  // ESP3D_USB_SERIAL_FEATURE

  // Serial Baud rate
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::serial_baud_rate);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_baud_rate);
    if (settingPtr) {
      uint32_t val =
          esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_baud_rate);
      std::string value = std::to_string(val);
      serial_baud_rate_label =
          listLine::add_label_to_line(value.c_str(), line_container, true);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

#if ESP3D_USB_SERIAL_FEATURE
  // USB serial Baud rate
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::usb_baud_rate);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr = esp3dTftsettings.getSettingPtr(
        ESP3DSettingIndex::esp3d_usb_serial_baud_rate);
    if (settingPtr) {
      uint32_t val = esp3dTftsettings.readUint32(
          ESP3DSettingIndex::esp3d_usb_serial_baud_rate);
      std::string value = std::to_string(val);
      usb_serial_baud_rate_label =
          listLine::add_label_to_line(value.c_str(), line_container, true);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }
#endif  // ESP3D_USB_SERIAL_FEATURE

  // Jog type
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::jog_type);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_jog_type);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_jog_type);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::relative)
                   : esp3dTranslationService.translate(ESP3DLabel::absolute);
      jog_type_label =
          listLine::add_label_to_line(value.c_str(), line_container, true);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  // Polling on
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::polling);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_polling_on);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_polling_on);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::disabled)
                   : esp3dTranslationService.translate(ESP3DLabel::enabled);
      polling_label =
          listLine::add_label_to_line(value.c_str(), line_container, true);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  // JSON
  //  show fan controls
  static ESP3DSettingIndex show_fan_controls_setting_index =
      ESP3DSettingIndex::esp3d_show_fan_controls;
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::fan_controls);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    show_fan_controls_label =
        listLine::add_label_to_line("", line_container, true);
    lv_obj_t *btnEdit =
        listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)&(show_fan_controls_setting_index));
  }

  // Auto level on
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::auto_leveling);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_auto_level_on);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_auto_level_on);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::disabled)
                   : esp3dTranslationService.translate(ESP3DLabel::enabled);
      auto_leveling_label =
          listLine::add_label_to_line(value.c_str(), line_container, true);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  // Bed width
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::bed_width);
  if (line_container) {
    std::string bed_width_str;
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_bed_width);
    if (settingPtr) {
      char out_str[15 + 1] = {0};
      bed_width_str = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_bed_width, out_str, 16);
    } else {
      esp3d_log_e("Failed to get bed width setting");
    }
    bed_width_label = listLine::add_label_to_line(bed_width_str.c_str(),
                                                  line_container, true);
    lv_obj_t *btnEdit =
        listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)(&(settingPtr->index)));
  }

  // Bed depth
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::bed_depth);
  if (line_container) {
    std::string bed_depth_str;
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_bed_depth);
    if (settingPtr) {
      char out_str[15 + 1] = {0};
      bed_depth_str = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_bed_depth, out_str, 16);
    } else {
      esp3d_log_e("Failed to get bed depth setting");
    }
    bed_depth_label = listLine::add_label_to_line(bed_depth_str.c_str(),
                                                  line_container, true);
    lv_obj_t *btnEdit =
        listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)(&(settingPtr->index)));
  }

  // Invert X axis
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::invert_axis, "X");
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_inverved_x);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_inverved_x);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::disabled)
                   : esp3dTranslationService.translate(ESP3DLabel::enabled);
      inverted_x_label =
          listLine::add_label_to_line(value.c_str(), line_container, true);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  // Invert Y axis
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::invert_axis, "Y");
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_inverved_y);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_inverved_y);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::disabled)
                   : esp3dTranslationService.translate(ESP3DLabel::enabled);
      inverted_y_label =
          listLine::add_label_to_line(value.c_str(), line_container, true);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  esp3dTftui.set_current_screen(ESP3DScreenType::settings);
  spinnerScreen::show_spinner();
  TaskHandle_t xHandle = NULL;
  BaseType_t res = xTaskCreatePinnedToCore(
      bgLoadJSONSettingsTask, "loadjsonsettingsTask", STACKDEPTH, NULL,
      TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Settings Task");
  } else {
    esp3d_log_e("Settings Task creation failed");
  }
}
}  // namespace settingsScreen