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

#include "screens/settings_screen.h"

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
#include "esp3d_hal.h"
#include "esp3d_lvgl.h"
#include "esp3d_settings.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "esp3d_tft_ui.h"
#include "filesystem/esp3d_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rendering/esp3d_rendering_client.h"
#include "screens/main_screen.h"
#include "screens/manual_leveling_screen.h"
#include "screens/menu_screen.h"
#include "tasks_def.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  Namespace
 **********************/
namespace settingsScreen {
#define STACKDEPTH 4096
#define TASKPRIORITY UI_TASK_PRIORITY - 1
#define TASKCORE UI_TASK_CORE

// static variables

lv_timer_t *settings_screen_delay_timer = NULL;
lv_timer_t *settings_screen_apply_timer = NULL;
lv_obj_t *ui_settings_list_ctl = NULL;
lv_obj_t *language_label = NULL;
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

// Static functions

/**
 * @brief Callback function for the delay timer in the settings screen.
 *
 * This function is called when the delay timer expires. It checks if the timer
 * is still valid and deletes it if necessary. Then, it creates the menu screen.
 *
 * @param timer Pointer to the timer object that triggered the callback.
 */
void settings_screen_delay_timer_cb(lv_timer_t *timer) {
  if (settings_screen_delay_timer &&
      lv_timer_is_valid(settings_screen_delay_timer)) {
    lv_timer_del(settings_screen_delay_timer);
  }
  settings_screen_delay_timer = NULL;
  menuScreen::create();
}

/**
 * @brief Callback function for updating the delay timer in the settings UI.
 *
 * This function is called when the delay timer needs to be updated in the
 * settings UI. It cancels any existing delay timer, initializes the translation
 * service, hides the spinner screen, and creates the settings UI.
 *
 * @param timer A pointer to the timer object that triggered the callback.
 */
void settings_ui_update_delay_timer_cb(lv_timer_t *timer) {
  if (settings_screen_delay_timer &&
      lv_timer_is_valid(settings_screen_delay_timer)) {
    lv_timer_del(settings_screen_delay_timer);
  }
  settings_screen_delay_timer = NULL;
  esp3dTranslationService.begin();
  spinnerScreen::hide();
  create();
}

/**
 * @brief Callback function to refresh the settings list.
 *
 * This function is called periodically to refresh the settings list on the
 * screen. It checks if a refresh is needed based on the user data passed
 * through the timer. If a refresh is needed, it creates the settings list on
 * the screen. If an apply timer is active, it deletes the timer. Finally, it
 * hides the spinner screen.
 *
 * @param timer The timer object that triggered the callback.
 */
void refresh_settings_list_cb(lv_timer_t *timer) {
  bool refresh = true;
  if (timer->user_data) {
    refresh = *(bool *)((timer->user_data));
  }
  if (settings_screen_apply_timer &&
      lv_timer_is_valid(settings_screen_apply_timer)) {
    lv_timer_del(settings_screen_apply_timer);
  }
  settings_screen_apply_timer = NULL;
  spinnerScreen::hide();
  if (refresh) create();
}

/**
 * @brief Background task to load JSON settings.
 *
 * This task is responsible for loading JSON settings in the background. It
 * reads specific settings from the esp3dTftJsonSettings object and updates the
 * corresponding UI elements accordingly.
 *
 * @param pvParameter A pointer to the task parameter (not used in this task).
 */
static void bgLoadJSONSettingsTask(void *pvParameter) {
  (void)pvParameter;
  esp3d_hal::wait(100);
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

/**
 * @brief Background task for saving JSON settings.
 *
 * This task is responsible for saving the JSON settings in the background.
 * It receives a pointer to an `ESP3DSettingsData` object as a parameter,
 * which contains the settings data to be saved. The task performs the necessary
 * operations to save the settings and update the user interface accordingly.
 *
 * @param pvParameter A pointer to the `ESP3DSettingsData` object.
 */
static void bgSaveJSONSettingsTask(void *pvParameter) {
  esp3d_hal::wait(100);
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
          mainScreen::show_fan_controls(true);
          if (data->label) {
            lv_label_set_text(data->label, esp3dTranslationService.translate(
                                               ESP3DLabel::enabled));
          }
        } else {
          mainScreen::show_fan_controls(false);
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
    msgBox::create(NULL, MsgBoxType::error, text.c_str());
  }

  if (!settings_screen_apply_timer) {
    settings_screen_apply_timer =
        lv_timer_create(refresh_settings_list_cb, 100, NULL);
  }
  vTaskDelete(NULL);
}

/**
 * @brief Creates a task to save the JSON settings data.
 *
 * This function creates a task to save the provided JSON settings data. It
 * shows a spinner screen while the task is running. The task is created with a
 * given name, stack depth, priority, and core. If the task creation is
 * successful, a log message is printed. Otherwise, an error log message is
 * printed.
 *
 * @param settingData A pointer to the ESP3DSettingsData object containing the
 * settings data to be saved.
 */
void CreateSaveJSONSettingTask(ESP3DSettingsData *settingData) {
  spinnerScreen::show();
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

/**
 * @brief Callback function called when a setting edit is done.
 *
 * This function is called when a setting edit is done and the user wants to
 * save the changes. It takes the edited string and the associated data as
 * parameters. The function saves the setting value and applies the changes if
 * necessary.
 *
 * @param str The edited string representing the new value of the setting.
 * @param data A pointer to the associated data for the setting.
 */
void setting_edit_done_cb(const char *str, void *data) {
  if (!data) {
    esp3d_log_e("No data provided");
    return;
  }
  esp3d_log("Saving setting to: %s\n", str);
  ESP3DSettingsData *settingData = (ESP3DSettingsData *)data;
  esp3d_log("Got index %d", (uint16_t)settingData->index);
  if (settingData->value != str) {
    esp3d_log("New value is different");
    // get value type
    const ESP3DSettingDescription *settingPtr = NULL;
    if (settingData->entry == "") {
      esp3d_log("Setting is not json setting");
      settingPtr = esp3dTftsettings.getSettingPtr(settingData->index);
    } else {
      esp3d_log("Setting is json setting");
    }
    if (settingPtr == NULL && settingData->entry == "") {
      esp3d_log_e("Unknown setting index %d", (uint16_t)settingData->index);
      return;
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
              case ESP3DSettingIndex::esp3d_ui_language:
                if (val_string == "English") {
                  val_string = "default";
                }
                break;
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
        case ESP3DSettingType::ip_t:
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
      msgBox::create(NULL, MsgBoxType::error, text.c_str());
      return;
    }
    if (!success_saving) {
      esp3d_log_e("Failed to save setting %s", str);
      std::string text =
          esp3dTranslationService.translate(ESP3DLabel::error_applying_setting);
      msgBox::create(NULL, MsgBoxType::error, text.c_str());
      return;
    }
    if (settingData->label) {
      lv_label_set_text(settingData->label, val_string.c_str());
    }
    // now apply the setting if needed
    switch (settingData->index) {
      case ESP3DSettingIndex::esp3d_ui_language:
        spinnerScreen::show();
        if (settings_screen_delay_timer) return;
        settings_screen_delay_timer =
            lv_timer_create(settings_ui_update_delay_timer_cb, 100, NULL);
        break;
      case ESP3DSettingIndex::esp3d_polling_on:
        renderingClient.setPolling(val_byte);
        break;
      case ESP3DSettingIndex::esp3d_inverved_x:
        manualLevelingScreen::invert_x(val_byte);
        break;
      case ESP3DSettingIndex::esp3d_inverved_y:
        manualLevelingScreen::invert_y(val_byte);
        break;
      case ESP3DSettingIndex::esp3d_auto_level_on:
        menuScreen::enable_auto_leveling(val_byte);
        break;
      case ESP3DSettingIndex::esp3d_bed_width:
        manualLevelingScreen::bed_width(strtod(val_string.c_str(), NULL));
        break;
      case ESP3DSettingIndex::esp3d_bed_depth:
        manualLevelingScreen::bed_depth(strtod(val_string.c_str(), NULL));
        break;
      default:
        break;
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

/**
 * @brief Callback function for editing a setting when a button is pressed.
 *
 * This function is called when a button is pressed to edit a setting. It
 * retrieves the necessary data for the setting based on the provided index and
 * displays an editor component for the setting. The user can then modify the
 * setting value and the changes are reflected in the UI.
 *
 * @param e The event object associated with the button press.
 */
void event_button_edit_setting_cb(lv_event_t *e) {
  esp3d_log("Show component editor");
  static ESP3DSettingsData data;
  data.index = *((ESP3DSettingIndex *)(lv_event_get_user_data(e)));
  data.choices.clear();
  data.entry = "";
  data.label = NULL;
  std::string title;
  uint8_t list_size = 0;
  esp3d_log("Got index %d", (uint16_t)data.index);
  switch (data.index) {
    case ESP3DSettingIndex::esp3d_ui_language:
      data.label = language_label;
      title = esp3dTranslationService.translate(ESP3DLabel::ui_language);
      // Translate default to english
      data.choices.push_back("English");  // english
      // search for ui_xxx.lng files in root
      if (flashFs.accessFS()) {
        DIR *dir = flashFs.opendir("/");
        if (dir) {
          struct dirent *entry;
          while ((entry = flashFs.readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
              continue;
            } else {
              std::string filename = entry->d_name;
              if (filename.length() > 4) {
                if (filename.substr(filename.length() - 4) == ".lng" &&
                    filename.substr(0, 3) == "ui_" && filename.length() != 7) {
                  data.choices.push_back(
                      filename.substr(3, filename.length() - 7));  // remove ui_
                                                                   // and .lng
                }
              }
            }
          }
          flashFs.closedir(dir);
        }
        flashFs.releaseFS();
      }
      break;

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
    choiceEditor::create(lv_scr_act(), data.value.c_str(), title.c_str(),
                         data.choices, setting_edit_done_cb, (void *)(&data));
  } else {
    // it is json setting so no getSettingPtr
    if (data.entry != "") {
      switch (data.index) {
        case ESP3DSettingIndex::esp3d_extensions:
          textEditor::create(lv_scr_act(), data.value.c_str(),
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
          textEditor::create(lv_scr_act(), data.value.c_str(),
                             setting_edit_done_cb, settingPtr->size, NULL,
                             false, (void *)(&data));
          break;
        case ESP3DSettingIndex::esp3d_bed_width:
        case ESP3DSettingIndex::esp3d_bed_depth:
          textEditor::create(lv_scr_act(), data.value.c_str(),
                             setting_edit_done_cb, 15, "0123456789.", true,
                             (void *)(&data));
          break;
        default:
          esp3d_log_e("Unknown setting index %d", (uint16_t)data.index);
          return;
      }
    }
  }
}

/**
 * Event handler for the back button in the settings screen.
 *
 * This function is called when the back button is clicked. It logs a message
 * and creates a timer if the button animation delay is enabled. If the delay is
 * not enabled, it directly calls the callback function.
 *
 * @param e The event object.
 */
void event_button_settings_back_handler(lv_event_t *e) {
  esp3d_log("back Clicked");
  if (ESP3D_BUTTON_ANIMATION_DELAY) {
    if (settings_screen_delay_timer) return;
    settings_screen_delay_timer = lv_timer_create(
        settings_screen_delay_timer_cb, ESP3D_BUTTON_ANIMATION_DELAY, NULL);
  } else {
    settings_screen_delay_timer_cb(NULL);
  }
}

/**
 * Creates the settings screen.
 * This function sets up the user interface for the settings screen, including
 * creating and configuring the necessary UI elements. It also loads the new
 * screen and deletes the old one.
 */
void create() {
  esp3dTftui.set_current_screen(ESP3DScreenType::none);
  // Screen creation
  esp3d_log("Settings screen creation");
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

  lv_obj_t *btnback = backButton::create(ui_new_screen);
  if (!lv_obj_is_valid(btnback)) {
    esp3d_log_e("Failed to create back button");
    return;
  }
  lv_obj_add_event_cb(btnback, event_button_settings_back_handler,
                      LV_EVENT_CLICKED, NULL);

  ui_settings_list_ctl = lv_list_create(ui_new_screen);
  if (!lv_obj_is_valid(ui_settings_list_ctl)) {
    esp3d_log_e("Failed to create list");
    return;
  }
  lv_obj_clear_flag(ui_settings_list_ctl, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_style_pad_left(ui_settings_list_ctl, ESP3D_LIST_CONTAINER_LR_PAD,
                            LV_PART_MAIN);
  lv_obj_set_style_pad_right(ui_settings_list_ctl, ESP3D_LIST_CONTAINER_LR_PAD,
                             LV_PART_MAIN);

  lv_obj_set_size(
      ui_settings_list_ctl, LV_HOR_RES - ESP3D_BUTTON_PRESSED_OUTLINE * 2,
      LV_VER_RES -
          ((3 * ESP3D_BUTTON_PRESSED_OUTLINE) + lv_obj_get_height(btnback)));

  lv_obj_set_pos(ui_settings_list_ctl, ESP3D_BUTTON_PRESSED_OUTLINE,
                 ESP3D_BUTTON_PRESSED_OUTLINE);
  lv_obj_t *line_container = NULL;
  std::string LabelStr = "";
  // Language
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::ui_language);
  if (line_container) {
    std::string ui_language;
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_ui_language);
    esp3d_log("Looking for index %d",
              (uint16_t)ESP3DSettingIndex::esp3d_ui_language);
    if (settingPtr) {
      esp3d_log("Found setting description for %d",
                (uint16_t)settingPtr->index);
      char out_str[(settingPtr->size) + 1] = {0};
      ui_language = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_ui_language, out_str, settingPtr->size);
    } else {
      esp3d_log_e("Failed to get setting description");
    }
    if (ui_language == "default") {
      ui_language = "English";
    }
    language_label =
        listLine::add_label(ui_language.c_str(), line_container, true);
    lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)(&(settingPtr->index)));
  }

  // Hostname
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::hostname);
  if (line_container) {
    std::string hostname;
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_hostname);
    if (settingPtr) {
      char out_str[(settingPtr->size) + 1] = {0};
      hostname = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_hostname,
                                             out_str, settingPtr->size);
    }
    hostname_label =
        listLine::add_label(hostname.c_str(), line_container, true);
    lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)(&(settingPtr->index)));
  }

  // Extensions
  static ESP3DSettingIndex extensions_setting_index =
      ESP3DSettingIndex::esp3d_extensions;
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::extensions);
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    extensions_label = listLine::add_label("", line_container, true);
    lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)&(extensions_setting_index));
  }

#if ESP3D_USB_SERIAL_FEATURE
  // USB Serial
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::output_client);
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
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
          listLine::add_label(value.c_str(), line_container, true);
      lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }
#endif  // ESP3D_USB_SERIAL_FEATURE

  // Serial Baud rate
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::serial_baud_rate);
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_baud_rate);
    if (settingPtr) {
      uint32_t val =
          esp3dTftsettings.readUint32(ESP3DSettingIndex::esp3d_baud_rate);
      std::string value = std::to_string(val);
      serial_baud_rate_label =
          listLine::add_label(value.c_str(), line_container, true);
      lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

#if ESP3D_USB_SERIAL_FEATURE
  // USB serial Baud rate
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::usb_baud_rate);
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr = esp3dTftsettings.getSettingPtr(
        ESP3DSettingIndex::esp3d_usb_serial_baud_rate);
    if (settingPtr) {
      uint32_t val = esp3dTftsettings.readUint32(
          ESP3DSettingIndex::esp3d_usb_serial_baud_rate);
      std::string value = std::to_string(val);
      usb_serial_baud_rate_label =
          listLine::add_label(value.c_str(), line_container, true);
      lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }
#endif  // ESP3D_USB_SERIAL_FEATURE

  // Jog type
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::jog_type);
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_jog_type);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_jog_type);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::relative)
                   : esp3dTranslationService.translate(ESP3DLabel::absolute);
      jog_type_label = listLine::add_label(value.c_str(), line_container, true);
      lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  // Polling on
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::polling);
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_polling_on);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_polling_on);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::disabled)
                   : esp3dTranslationService.translate(ESP3DLabel::enabled);
      polling_label = listLine::add_label(value.c_str(), line_container, true);
      lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  // JSON
  //  show fan controls
  static ESP3DSettingIndex show_fan_controls_setting_index =
      ESP3DSettingIndex::esp3d_show_fan_controls;
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::fan_controls);
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    show_fan_controls_label = listLine::add_label("", line_container, true);
    lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)&(show_fan_controls_setting_index));
  }

  // Auto level on
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::auto_leveling);
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_auto_level_on);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_auto_level_on);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::disabled)
                   : esp3dTranslationService.translate(ESP3DLabel::enabled);
      auto_leveling_label =
          listLine::add_label(value.c_str(), line_container, true);
      lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  // Bed width
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::bed_width);
  if (line_container) {
    std::string bed_width_str;
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_bed_width);
    if (settingPtr) {
      char out_str[15 + 1] = {0};
      bed_width_str = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_bed_width, out_str, 16);
    } else {
      esp3d_log_e("Failed to get bed width setting");
    }
    bed_width_label =
        listLine::add_label(bed_width_str.c_str(), line_container, true);
    lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)(&(settingPtr->index)));
  }

  // Bed depth
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::bed_depth);
  if (line_container) {
    std::string bed_depth_str;
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_bed_depth);
    if (settingPtr) {
      char out_str[15 + 1] = {0};
      bed_depth_str = esp3dTftsettings.readString(
          ESP3DSettingIndex::esp3d_bed_depth, out_str, 16);
    } else {
      esp3d_log_e("Failed to get bed depth setting");
    }
    bed_depth_label =
        listLine::add_label(bed_depth_str.c_str(), line_container, true);
    lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb, LV_EVENT_CLICKED,
                        (void *)(&(settingPtr->index)));
  }

  // Invert X axis
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::invert_axis, "X");
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_inverved_x);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_inverved_x);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::disabled)
                   : esp3dTranslationService.translate(ESP3DLabel::enabled);
      inverted_x_label =
          listLine::add_label(value.c_str(), line_container, true);
      lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  // Invert Y axis
  line_container = listLine::create(ui_settings_list_ctl);
  if (!lv_obj_is_valid(line_container)) {
    esp3d_log_e("Failed to create line container");
    return;
  }
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::invert_axis, "Y");
  if (line_container) {
    listLine::add_label(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_inverved_y);
    if (settingPtr) {
      uint8_t val =
          esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_inverved_y);
      std::string value =
          val == 0 ? esp3dTranslationService.translate(ESP3DLabel::disabled)
                   : esp3dTranslationService.translate(ESP3DLabel::enabled);
      inverted_y_label =
          listLine::add_label(value.c_str(), line_container, true);
      lv_obj_t *btnEdit = listLine::add_button(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_setting_cb,
                          LV_EVENT_CLICKED, (void *)(&(settingPtr->index)));
    }
  }

  esp3dTftui.set_current_screen(ESP3DScreenType::settings);
  spinnerScreen::show();
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