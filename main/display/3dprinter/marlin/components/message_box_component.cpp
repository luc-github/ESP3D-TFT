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

#include "message_box_component.h"

#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_styles.h"
#include "translations/esp3d_translation_service.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
namespace msgBox {

const char *btn_simple[] = {LV_SYMBOL_OK, ""};
const char *btn_ok_cancel[] = {LV_SYMBOL_OK, LV_SYMBOL_CLOSE, ""};
char **btns;

void event_msgbox_cb(lv_event_t *e) {
  lv_obj_t *mbox = lv_event_get_current_target(e);
  // MsgBoxType *type = (MsgBoxType *)lv_event_get_user_data(e);
  std::string rep = lv_msgbox_get_active_btn_text(mbox);
  lv_msgbox_close(mbox);
}

lv_obj_t *messageBoxMain(lv_obj_t *container, MsgBoxType type,
                         const char *content) {
  std::string title;
  switch (type) {
    case MsgBoxType::error:
      title = esp3dTranslationService.translate(ESP3DLabel::error);
      btns = (char **)btn_simple;
      break;
    case MsgBoxType::information:
      title = esp3dTranslationService.translate(ESP3DLabel::information);
      btns = (char **)btn_simple;
      break;
    case MsgBoxType::confirmation:
      title = esp3dTranslationService.translate(ESP3DLabel::confirmation);
      btns = (char **)btn_ok_cancel;
      break;
    default:
      esp3d_log("Message Box type unknown");
      return nullptr;
  };

  lv_obj_t *mbox =
      lv_msgbox_create(NULL, title.c_str(), content, (const char **)btns, true);
  apply_style(mbox, ESP3DStyleType::message_box);
  lv_obj_center(mbox);
  return mbox;
}

lv_obj_t *messageBox(lv_obj_t *container, MsgBoxType type,
                     const char *content) {
  lv_obj_t *mbox = messageBoxMain(container, type, content);
  lv_obj_add_event_cb(mbox, event_msgbox_cb, LV_EVENT_VALUE_CHANGED,
                      (void *)&type);
  return mbox;
}
lv_obj_t *confirmationBox(lv_obj_t *container, MsgBoxType type,
                          const char *content, lv_event_cb_t event_cb) {
  lv_obj_t *mbox = messageBoxMain(container, type, content);
  lv_obj_add_event_cb(mbox, event_cb, LV_EVENT_VALUE_CHANGED, (void *)&type);
  return mbox;
}
}  // namespace msgBox
