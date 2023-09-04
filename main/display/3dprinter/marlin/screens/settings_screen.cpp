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

struct ESP3DJSONSettingsData {
  std::string entry = "";
  std::string value = "";
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
  ESP3DJSONSettingsData *data = (ESP3DJSONSettingsData *)pvParameter;
  esp3d_log("Got value %s in task", data->value.c_str());
  // do the change

  esp3d_log("Value %s is valid", data->value.c_str());
  if (esp3dTftJsonSettings.writeString("settings", data->entry.c_str(),
                                       data->value.c_str())) {
    if (data->entry == "filesfilter") {
      if (extensions_label) {
        lv_label_set_text(extensions_label, data->value.c_str());
      }
    }
    if (data->entry == "showfanctrls") {
      if (show_fan_controls_label) {
        lv_label_set_text(show_fan_controls_label, data->value.c_str());
      }
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

// serial_baud_rate_edit_done_cb
void serial_baud_rate_edit_done_cb(const char *str) {
  esp3d_log("Saving baud rate to: %s\n", str);
  if (strcmp(str, lv_label_get_text(serial_baud_rate_label)) != 0) {
    uint32_t val = atoi(str);
    if (esp3dTftsettings.isValidIntegerSetting(
            val, ESP3DSettingIndex::esp3d_baud_rate)) {
      esp3d_log("Value %s is valid", str);
      if (esp3dTftsettings.writeUint32(ESP3DSettingIndex::esp3d_baud_rate,
                                       val)) {
        if (serial_baud_rate_label) {
          lv_label_set_text(serial_baud_rate_label, str);
        }
      } else {
        esp3d_log_e("Failed to save output client");
        std::string text = esp3dTranslationService.translate(
            ESP3DLabel::error_applying_setting);
        msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      }
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// usb_serial_baud_rate_edit_done_cb
void usb_serial_baud_rate_edit_done_cb(const char *str) {
  esp3d_log("Saving usb baud rate to: %s\n", str);
  if (strcmp(str, lv_label_get_text(usb_serial_baud_rate_label)) != 0) {
    uint32_t val = atoi(str);
    if (esp3dTftsettings.isValidIntegerSetting(
            val, ESP3DSettingIndex::esp3d_usb_serial_baud_rate)) {
      esp3d_log("Value %s is valid", str);
      if (esp3dTftsettings.writeUint32(
              ESP3DSettingIndex::esp3d_usb_serial_baud_rate, val)) {
        if (usb_serial_baud_rate_label) {
          lv_label_set_text(usb_serial_baud_rate_label, str);
        }
      } else {
        esp3d_log_e("Failed to save output client");
        std::string text = esp3dTranslationService.translate(
            ESP3DLabel::error_applying_setting);
        msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      }
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// outputclient_edit_done_cb
void outputclient_edit_done_cb(const char *str) {
  esp3d_log("Saving output client to: %s\n", str);

  if (strcmp(str, lv_label_get_text(output_client_label)) != 0) {
    uint8_t val =
        (uint8_t)ESP3DClientType::no_client;  // default value if not found
    if (strcmp(str, esp3dTranslationService.translate(ESP3DLabel::usb)) == 0) {
      val = (uint8_t)ESP3DClientType::usb_serial;
    } else if (strcmp(str, esp3dTranslationService.translate(
                               ESP3DLabel::serial)) == 0) {
      val = (uint8_t)ESP3DClientType::serial;
    }

    if (esp3dTftsettings.isValidByteSetting(
            val, ESP3DSettingIndex::esp3d_output_client)) {
      esp3d_log("Value %s is valid", str);
      if (esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_output_client,
                                     val)) {
        if (output_client_label) {
          lv_label_set_text(output_client_label, str);
        }
      } else {
        esp3d_log_e("Failed to save output client");
        std::string text = esp3dTranslationService.translate(
            ESP3DLabel::error_applying_setting);
        msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      }
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// jog_type_edit_done_cb
void jog_type_edit_done_cb(const char *str) {
  esp3d_log("Saving jog type to: %s\n", str);

  if (strcmp(str, lv_label_get_text(jog_type_label)) != 0) {
    uint8_t val =
        (uint8_t)ESP3DClientType::no_client;  // default value if not found
    if (strcmp(str, esp3dTranslationService.translate(ESP3DLabel::absolute)) ==
        0) {
      val = 1;
    } else if (strcmp(str, esp3dTranslationService.translate(
                               ESP3DLabel::relative)) == 0) {
      val = 0;
    }

    if (esp3dTftsettings.isValidByteSetting(
            val, ESP3DSettingIndex::esp3d_jog_type)) {
      esp3d_log("Value %s is valid", str);
      if (esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_jog_type, val)) {
        if (jog_type_label) {
          lv_label_set_text(jog_type_label, str);
        }
      } else {
        esp3d_log_e("Failed to save jog type");
        std::string text = esp3dTranslationService.translate(
            ESP3DLabel::error_applying_setting);
        msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      }
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// polling_edit_done_cb
void polling_edit_done_cb(const char *str) {
  esp3d_log("Saving polling state to: %s\n", str);

  if (strcmp(str, lv_label_get_text(polling_label)) != 0) {
    uint8_t val =
        (uint8_t)ESP3DClientType::no_client;  // default value if not found
    if (strcmp(str, esp3dTranslationService.translate(ESP3DLabel::enabled)) ==
        0) {
      val = 1;
    } else if (strcmp(str, esp3dTranslationService.translate(
                               ESP3DLabel::disabled)) == 0) {
      val = 0;
    }

    if (esp3dTftsettings.isValidByteSetting(
            val, ESP3DSettingIndex::esp3d_polling_on)) {
      esp3d_log("Value %s is valid", str);
      if (esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_polling_on,
                                     val)) {
        renderingClient.setPolling(val);

        if (polling_label) {
          lv_label_set_text(polling_label, str);
        }
      } else {
        esp3d_log_e("Failed to save polling mode");
        std::string text = esp3dTranslationService.translate(
            ESP3DLabel::error_applying_setting);
        msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      }
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// auto_levling_edit_done_cb
void auto_leveling_edit_done_cb(const char *str) {
  esp3d_log("Saving auto leveling state to: %s\n", str);

  if (strcmp(str, lv_label_get_text(auto_leveling_label)) != 0) {
    uint8_t val =
        (uint8_t)ESP3DClientType::no_client;  // default value if not found
    if (strcmp(str, esp3dTranslationService.translate(ESP3DLabel::enabled)) ==
        0) {
      val = 1;
    } else if (strcmp(str, esp3dTranslationService.translate(
                               ESP3DLabel::disabled)) == 0) {
      val = 0;
    }

    if (esp3dTftsettings.isValidByteSetting(
            val, ESP3DSettingIndex::esp3d_auto_level_on)) {
      esp3d_log("Value %s is valid", str);
      if (esp3dTftsettings.writeByte(ESP3DSettingIndex::esp3d_auto_level_on,
                                     val)) {
        menuScreen::enable_auto_leveling(val == 1 ? true : false);
        if (auto_leveling_label) {
          lv_label_set_text(auto_leveling_label, str);
        }
      } else {
        esp3d_log_e("Failed to save auto leveling mode");
        std::string text = esp3dTranslationService.translate(
            ESP3DLabel::error_applying_setting);
        msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      }
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// hostname_edit_done_cb
void hostname_edit_done_cb(const char *str) {
  esp3d_log("Saving hostname to: %s\n", str);
  if (strcmp(str, lv_label_get_text(hostname_label)) != 0) {
    if (esp3dTftsettings.isValidStringSetting(
            str, ESP3DSettingIndex::esp3d_hostname)) {
      esp3d_log("Value %s is valid", str);
      if (esp3dTftsettings.writeString(ESP3DSettingIndex::esp3d_hostname,
                                       str)) {
        if (hostname_label) {
          lv_label_set_text(hostname_label, str);
        }
      } else {
        esp3d_log_e("Failed to save hostname");
        std::string text = esp3dTranslationService.translate(
            ESP3DLabel::error_applying_setting);
        msgBox::messageBox(NULL, MsgBoxType::error, text.c_str());
      }
    }
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

void CreateSaveSettingTask(const char *entry, const char *value) {
  spinnerScreen::show_spinner();
  TaskHandle_t xHandle = NULL;
  static ESP3DJSONSettingsData data;
  data.entry = entry;
  data.value = value ? value : "";
  BaseType_t res = xTaskCreatePinnedToCore(
      bgSaveJSONSettingsTask, "savesettingsTask", STACKDEPTH, (void *)(&data),
      TASKPRIORITY, &xHandle, TASKCORE);
  if (res == pdPASS && xHandle) {
    esp3d_log("Created Settings Task");
  } else {
    esp3d_log_e("Settings Task creation failed");
  }
}

// extensions_edit_done_cb
void extensions_edit_done_cb(const char *str) {
  esp3d_log("Saving extensions to: %s\n", str);
  if (strcmp(str, lv_label_get_text(extensions_label)) != 0) {
    CreateSaveSettingTask("filesfilter", str);
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// showfanctrls_edit_done_cb
void showfanctrls_edit_done_cb(const char *str) {
  esp3d_log("Saving Show fan controls to: %s\n", str);
  if (strcmp(str, lv_label_get_text(show_fan_controls_label)) != 0) {
    // now convert to boolean string
    std::string value = "false";
    if (strcmp(str, esp3dTranslationService.translate(ESP3DLabel::enabled)) ==
        0) {
      value = "true";
      mainScreen::update_show_fan_controls(true);
    } else {
      mainScreen::update_show_fan_controls(false);
    }
    CreateSaveSettingTask("showfanctrls", value.c_str());
  } else {
    esp3d_log("New value is identical do not save it");
  }
}

// event_button_edit_output_client_cb
void event_button_edit_output_client_cb(lv_event_t *e) {
  esp3d_log("Show component output client editor");
  const char *text = (const char *)lv_event_get_user_data(e);
  std::list<std::string> choices;
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::serial));
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::usb));
  std::string title =
      esp3dTranslationService.translate(ESP3DLabel::output_client);
  choiceEditor::create_choice_editor(lv_scr_act(), text, title.c_str(), choices,
                                     outputclient_edit_done_cb);
}

// event_button_edit_usb_serial_baud_rate_cb
void event_button_edit_usb_serial_baud_rate_cb(lv_event_t *e) {
  esp3d_log("Show component usb serial baud rate editor");
  const char *text = (const char *)lv_event_get_user_data(e);
  std::list<std::string> choices;
  uint8_t list_size = sizeof(SupportedBaudList) / sizeof(uint32_t);
  for (uint8_t i = 0; i < list_size; i++) {
    choices.push_back(std::to_string(SupportedBaudList[i]));
  }
  std::string title =
      esp3dTranslationService.translate(ESP3DLabel::usb_baud_rate);

  choiceEditor::create_choice_editor(lv_scr_act(), text, title.c_str(), choices,
                                     usb_serial_baud_rate_edit_done_cb);
}

// event_button_edit_serial_baud_rate_cb
void event_button_edit_serial_baud_rate_cb(lv_event_t *e) {
  esp3d_log("Show component serial baud rate editor");
  const char *text = (const char *)lv_event_get_user_data(e);
  std::list<std::string> choices;
  uint8_t list_size = sizeof(SupportedBaudList) / sizeof(uint32_t);
  for (uint8_t i = 0; i < list_size; i++) {
    choices.push_back(std::to_string(SupportedBaudList[i]));
  }
  std::string title =
      esp3dTranslationService.translate(ESP3DLabel::serial_baud_rate);
  choiceEditor::create_choice_editor(lv_scr_act(), text, title.c_str(), choices,
                                     serial_baud_rate_edit_done_cb);
}

// event_button_edit_jog_type_cb
void event_button_edit_jog_type_cb(lv_event_t *e) {
  esp3d_log("Show component jog type editor");
  const char *text = (const char *)lv_event_get_user_data(e);
  std::list<std::string> choices;
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::relative));
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::absolute));
  std::string title = esp3dTranslationService.translate(ESP3DLabel::jog_type);
  choiceEditor::create_choice_editor(lv_scr_act(), text, title.c_str(), choices,
                                     jog_type_edit_done_cb);
}

// event_button_edit_polling_cb
void event_button_edit_polling_cb(lv_event_t *e) {
  esp3d_log("Show component polling editor");
  const char *text = (const char *)lv_event_get_user_data(e);
  std::list<std::string> choices;
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::disabled));
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::enabled));
  std::string title = esp3dTranslationService.translate(ESP3DLabel::polling);
  choiceEditor::create_choice_editor(lv_scr_act(), text, title.c_str(), choices,
                                     polling_edit_done_cb);
}

// event_button_edit_auto_leveling_cb
void event_button_edit_auto_leveling_cb(lv_event_t *e) {
  esp3d_log("Show component eveling editor");
  const char *text = (const char *)lv_event_get_user_data(e);
  std::list<std::string> choices;
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::disabled));
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::enabled));
  std::string title =
      esp3dTranslationService.translate(ESP3DLabel::auto_leveling);
  choiceEditor::create_choice_editor(lv_scr_act(), text, title.c_str(), choices,
                                     auto_leveling_edit_done_cb);
}

// event_button_edit_extensions_cb
void event_button_edit_extensions_cb(lv_event_t *e) {
  esp3d_log("Show component");
  const char *text = (const char *)lv_label_get_text(extensions_label);
  textEditor::create_text_editor(lv_scr_act(), text, extensions_edit_done_cb);
}

// event_button_edit_show_fan_controls_cb
void event_button_edit_show_fan_controls_cb(lv_event_t *e) {
  esp3d_log("Show component");
  const char *text = (const char *)lv_label_get_text(show_fan_controls_label);
  std::list<std::string> choices;
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::disabled));
  choices.push_back(esp3dTranslationService.translate(ESP3DLabel::enabled));
  std::string title =
      esp3dTranslationService.translate(ESP3DLabel::fan_controls);
  choiceEditor::create_choice_editor(lv_scr_act(), text, title.c_str(), choices,
                                     showfanctrls_edit_done_cb);
}

// event_button_edit_hostname_cb
void event_button_edit_hostname_cb(lv_event_t *e) {
  esp3d_log("Show component");
  const char *text = (const char *)lv_event_get_user_data(e);
  textEditor::create_text_editor(lv_scr_act(), text, hostname_edit_done_cb);
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

  lv_obj_set_size(
      ui_settings_list_ctl, LV_HOR_RES - CURRENT_BUTTON_PRESSED_OUTLINE * 2,
      LV_VER_RES -
          ((3 * CURRENT_BUTTON_PRESSED_OUTLINE) + lv_obj_get_height(btnback)));

  lv_obj_set_pos(ui_settings_list_ctl, CURRENT_BUTTON_PRESSED_OUTLINE,
                 CURRENT_BUTTON_PRESSED_OUTLINE);
  // Hostname
  lv_obj_t *line_container =
      listLine::create_list_line_container(ui_settings_list_ctl);
  std::string LabelStr =
      esp3dTranslationService.translate(ESP3DLabel::hostname);
  if (line_container) {
    std::string hostname;
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    const ESP3DSettingDescription *settingPtr =
        esp3dTftsettings.getSettingPtr(ESP3DSettingIndex::esp3d_hostname);
    if (settingPtr) {
      char out_str[33] = {0};
      hostname = esp3dTftsettings.readString(ESP3DSettingIndex::esp3d_hostname,
                                             out_str, settingPtr->size);
    }
    hostname_label =
        listLine::add_label_to_line(hostname.c_str(), line_container, false);
    lv_obj_t *btnEdit =
        listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_hostname_cb,
                        LV_EVENT_CLICKED,
                        (void *)(lv_label_get_text(hostname_label)));
  }

  // Extensions
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::extensions);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    extensions_label = listLine::add_label_to_line("", line_container, false);
    lv_obj_t *btnEdit =
        listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_extensions_cb,
                        LV_EVENT_CLICKED, NULL);
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
          listLine::add_label_to_line(value.c_str(), line_container, false);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_output_client_cb,
                          LV_EVENT_CLICKED,
                          (void *)(lv_label_get_text(output_client_label)));
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
          listLine::add_label_to_line(value.c_str(), line_container, false);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_serial_baud_rate_cb,
                          LV_EVENT_CLICKED,
                          (void *)(lv_label_get_text(serial_baud_rate_label)));
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
          listLine::add_label_to_line(value.c_str(), line_container, false);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(
          btnEdit, event_button_edit_usb_serial_baud_rate_cb, LV_EVENT_CLICKED,
          (void *)(lv_label_get_text(usb_serial_baud_rate_label)));
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
          listLine::add_label_to_line(value.c_str(), line_container, false);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_jog_type_cb,
                          LV_EVENT_CLICKED,
                          (void *)(lv_label_get_text(jog_type_label)));
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
          listLine::add_label_to_line(value.c_str(), line_container, false);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_polling_cb,
                          LV_EVENT_CLICKED,
                          (void *)(lv_label_get_text(polling_label)));
    }
  }

  // show fan controls
  line_container = listLine::create_list_line_container(ui_settings_list_ctl);
  LabelStr = esp3dTranslationService.translate(ESP3DLabel::fan_controls);
  if (line_container) {
    listLine::add_label_to_line(LabelStr.c_str(), line_container, true);
    show_fan_controls_label =
        listLine::add_label_to_line("", line_container, false);
    lv_obj_t *btnEdit =
        listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
    lv_obj_add_event_cb(btnEdit, event_button_edit_show_fan_controls_cb,
                        LV_EVENT_CLICKED, NULL);
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
          listLine::add_label_to_line(value.c_str(), line_container, false);
      lv_obj_t *btnEdit =
          listLine::add_button_to_line(LV_SYMBOL_EDIT, line_container);
      lv_obj_add_event_cb(btnEdit, event_button_edit_auto_leveling_cb,
                          LV_EVENT_CLICKED,
                          (void *)(lv_label_get_text(auto_leveling_label)));
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