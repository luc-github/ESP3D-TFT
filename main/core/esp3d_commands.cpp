/*
  esp3d_commands class
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

#include "esp3d_commands.h"

#include "esp3d_settings.h"
#include "http/esp3d_http_service.h"
#include "serial/esp3d_serial_client.h"
#include "websocket/esp3d_webui_service.h"
#include "websocket/esp3d_ws_service.h"

#if ESP3D_TELNET_FEATURE
#include "socket_server/esp3d_socket_server.h"
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial/esp3d_usb_serial_client.h"
#endif  // #if ESP3D_USB_SERIAL_FEATURE
#include <stdio.h>

#include <string>

#include "esp3d_string.h"

Esp3DCommands esp3dCommands;

Esp3DCommands::Esp3DCommands() { _output_client = Esp3dClient::serial; }
Esp3DCommands::~Esp3DCommands() {}
bool Esp3DCommands::is_esp_command(uint8_t* sbuf, size_t len) {
  if (len < 5) {
    return false;
  }
  // normal command header
  if ((char(sbuf[0]) == '[') && (char(sbuf[1]) == 'E') &&
      (char(sbuf[2]) == 'S') && (char(sbuf[3]) == 'P') &&
      ((char(sbuf[4]) == ']') || (char(sbuf[5]) == ']') ||
       (char(sbuf[6]) == ']') || (char(sbuf[7]) == ']'))) {
    return true;
  }
  // echo command header on some targeted firmware
  if (len >= 14) {
    if ((char(sbuf[0]) == 'e') && (char(sbuf[1]) == 'c') &&
        (char(sbuf[2]) == 'h') && (char(sbuf[3]) == 'o') &&
        (char(sbuf[4]) == ':') && (char(sbuf[5]) == ' ') &&
        (char(sbuf[6]) == '[') && (char(sbuf[7]) == 'E') &&
        (char(sbuf[8]) == 'S') && (char(sbuf[9]) == 'P') &&
        ((char(sbuf[4]) == ']') || (char(sbuf[5]) == ']') ||
         (char(sbuf[6]) == ']') || (char(sbuf[7]) == ']'))) {
      return true;
    }
  }
  return false;
}

void Esp3DCommands::process(Esp3dMessage* msg) {
  static bool lastIsESP3D = false;
  if (!msg) {
    return;
  }
  if (is_esp_command(msg->data, msg->size)) {
    esp3d_log("Detected ESP command");
    lastIsESP3D = true;
    uint cmdId = 0;
    uint espcmdpos = 0;
    bool startcmd = false;
    bool endcmd = false;
    for (uint i = 0; i < msg->size && espcmdpos == 0; i++) {
      if (char(msg->data[i]) == ']') {  // start command flag
        endcmd = true;
        espcmdpos = i + 1;
      } else if (char(msg->data[i]) == '[') {  // end command flag
        startcmd = true;
      } else if (startcmd && !endcmd &&
                 std::isdigit(static_cast<unsigned char>(
                     char(msg->data[i])))) {  // command id
        if (cmdId != 0) {
          cmdId = (cmdId * 10);
        }
        cmdId += (msg->data[i] - 48);
      }
    }
    // execute esp command
    execute_internal_command(cmdId, espcmdpos, msg);
  } else {
    esp3d_log("Dispatch command, len %d, to %d", msg->size, msg->target);

    // Work around to avoid to dispatch single \n or \r to everyone as it is
    // part of previous ESP3D command
    if (msg->size == 1 &&
        ((char(msg->data[0]) == '\n') || (char(msg->data[0]) == '\r')) &&
        lastIsESP3D) {
      lastIsESP3D = false;
      // delete message
      Esp3DClient::deleteMsg(msg);
      return;
    }
    lastIsESP3D = false;
    dispatch(msg);
  }
}

bool Esp3DCommands::dispatchSetting(bool json, const char* filter,
                                    esp3d_setting_index_t index,
                                    const char* help, const char** optionValues,
                                    const char** optionLabels, uint32_t maxsize,
                                    uint32_t minsize, uint32_t minsize2,
                                    uint8_t precision, const char* unit,
                                    bool needRestart, Esp3dClient target,
                                    esp3d_request_t requestId, bool isFirst) {
  std::string tmpstr;
  std::string value;
  char out_str[255];
  tmpstr.reserve(
      350);  // to save time and avoid several memories allocation delay
  const esp3d_setting_desc_t* elementSetting =
      esp3dTFTsettings.getSettingPtr(index);
  if (!elementSetting) {
    return false;
  }
  switch (elementSetting->type) {
    case Esp3dSettingType::byte_t:
      value = std::to_string(esp3dTFTsettings.readByte(index));
      break;
    case Esp3dSettingType::integer_t:
      value = std::to_string(esp3dTFTsettings.readUint32(index));
      break;
    case Esp3dSettingType::ip:
      value = esp3dTFTsettings.readIPString(index);
      break;
    case Esp3dSettingType::float_t:
      // TODO Add float support ?
      value = "Not supported";
      break;
    case Esp3dSettingType::mask:
      // TODO Add Mask support ?
      value = "Not supported";
      break;
    case Esp3dSettingType::bitsfield:
      // TODO Add bitfield support ?
      value = "Not supported";
      break;
    default:  // String
      if (index == esp3d_sta_password || index == esp3d_ap_password ||
#if ESP3D_NOTIFICATIONS_FEATURE
          index == esp3d_notification_token_1 ||
          index == esp3d_notification_token_2 ||
#endif  // ESP3D_NOTIFICATIONS_FEATURE

          index == esp3d_admin_password ||
          index == esp3d_user_password) {  // hide passwords using  ********
        value = HIDDEN_SETTING_VALUE;
      } else {
        value =
            esp3dTFTsettings.readString(index, out_str, elementSetting->size);
      }
  }
  if (json) {
    if (!isFirst) {
      tmpstr += ",";
    }
    tmpstr += "{\"F\":\"";
    tmpstr += filter;
    tmpstr += "\",\"P\":\"";
    tmpstr += std::to_string(index);
    tmpstr += "\",\"T\":\"";
    switch (elementSetting->type) {
      case Esp3dSettingType::byte_t:
        tmpstr += "B";
        break;
      case Esp3dSettingType::integer_t:
        tmpstr += "I";
        break;
      case Esp3dSettingType::ip:
        tmpstr += "A";
        break;
      case Esp3dSettingType::float_t:
        tmpstr += "F";
        break;
      case Esp3dSettingType::mask:
        tmpstr += "M";
        break;
      case Esp3dSettingType::bitsfield:
        tmpstr += "X";
        break;
      default:
        tmpstr += "S";
    }
    tmpstr += "\",\"V\":\"";
    tmpstr += value;
    tmpstr += "\",\"H\":\"";
    tmpstr += help;
    tmpstr += "\"";
    if (needRestart) {
      tmpstr += ",\"R\":\"1\"";
    }
    if (optionValues && optionLabels) {
      tmpstr += ",\"O\":[";
      for (uint8_t i = 0; i < maxsize; i++) {
        if (i > 0) {
          tmpstr += ",";
        }
        tmpstr += "{\"";
        // be sure we have same size for both array to avoid overflow
        tmpstr += optionLabels[i];
        tmpstr += "\":\"";
        tmpstr += optionValues[i];
        tmpstr += "\"}";
      }
      tmpstr += "]";
    }
    if (unit) {
      tmpstr += ",\"R\":\"";
      tmpstr += unit;
      tmpstr += "\"";
    }
    if (precision != ((uint8_t)-1)) {
      tmpstr += ",\"E\":\"";
      tmpstr += std::to_string(precision);
      tmpstr += "\"";
    }
    if (maxsize != (uint32_t)-1 && !optionValues) {
      tmpstr += ",\"S\":\"";
      tmpstr += std::to_string(maxsize);
      tmpstr += "\"";
    }
    if (minsize != (uint32_t)-1) {
      tmpstr += ",\"M\":\"";
      tmpstr += std::to_string(minsize);
      tmpstr += "\"";
    }
    if (minsize2 != (uint32_t)-1) {
      tmpstr += ",\"MS\":\"";
      tmpstr += std::to_string(minsize2);
      tmpstr += "\"";
    }
    tmpstr += "}";
  } else {
    tmpstr = filter;
    tmpstr += "/";
    tmpstr += help;
    tmpstr += ": ";
    tmpstr += value;
    tmpstr += "\n";
  }
  return dispatch(tmpstr.c_str(), target, requestId, Esp3dMessageType::core);
}

bool Esp3DCommands::dispatchAuthenticationError(Esp3dMessage* msg, uint cmdid,
                                                bool json) {
  std::string tmpstr;
  if (!msg) {
    return false;
  }
  msg->authentication_level = Esp3dAuthenticationLevel::not_authenticated;
#if ESP3D_HTTP_FEATURE
  if (msg->target == Esp3dClient::webui && msg->requestId.httpReq) {
    httpd_resp_set_status(msg->requestId.httpReq, "401 UNAUTHORIZED");
  }
#endif  // ESP3D_HTTP_FEATURE
  // answer is one message, override for safety
  msg->type = Esp3dMessageType::unique;
  if (json) {
    tmpstr = "{\"cmd\":\"";
    tmpstr += std::to_string(cmdid);
    tmpstr +=
        "\",\"status\":\"error\",\"data\":\"Wrong authentication level\"}";
  } else {
    tmpstr = "Wrong authentication level\n";
  }
  return dispatch(msg, tmpstr.c_str());
}

bool Esp3DCommands::dispatchAnswer(Esp3dMessage* msg, uint cmdid, bool json,
                                   bool hasError, const char* answerMsg) {
  std::string tmpstr;
  if (!msg || !answerMsg) {
    esp3d_log_e("no msg");
    return false;
  }
  // answer is one message, override for safety
  msg->type = Esp3dMessageType::unique;
  if (json) {
    tmpstr = "{\"cmd\":\"" + std::to_string(cmdid) + "\",\"status\":\"";

    if (hasError) {
      tmpstr += "error";
    } else {
      tmpstr += "ok";
    }
    tmpstr += "\",\"data\":";
    if (answerMsg[0] != '{') {
      tmpstr += "\"";
    }
    tmpstr += answerMsg;
    if (answerMsg[0] != '{') {
      tmpstr += "\"";
    }
    tmpstr += "}\n";
  } else {
    tmpstr = answerMsg;
    tmpstr += "\n";
  }
  return dispatch(msg, tmpstr.c_str());
}

bool Esp3DCommands::dispatchKeyValue(bool json, const char* key,
                                     const char* value, Esp3dClient target,
                                     esp3d_request_t requestId, bool nested,
                                     bool isFirst) {
  std::string tmpstr = "";
  if (json) {
    if (!isFirst) {
      tmpstr += ",";
    }
    if (nested) {
      tmpstr += "{";
    }
    tmpstr += "\"";
  }
  tmpstr += key;
  if (json) {
    tmpstr += "\":\"";
  } else {
    tmpstr += ": ";
  }
  tmpstr += value;
  if (json) {
    tmpstr += "\"";
    if (nested) {
      tmpstr += "}";
    }
  } else {
    tmpstr += "\n";
  }
  return dispatch(tmpstr.c_str(), target, requestId, Esp3dMessageType::core);
}

bool Esp3DCommands::dispatchIdValue(bool json, const char* Id,
                                    const char* value, Esp3dClient target,
                                    esp3d_request_t requestId, bool isFirst) {
  std::string tmpstr = "";
  if (json) {
    if (!isFirst) {
      tmpstr += ",";
    }
    tmpstr += "{\"id\":\"";
  }
  tmpstr += Id;
  if (json) {
    tmpstr += "\",\"value\":\"";
  } else {
    tmpstr += ": ";
  }
  tmpstr += value;
  if (json) {
    tmpstr += "\"}";
  } else {
    tmpstr += "\n";
  }
  return dispatch(tmpstr.c_str(), target, requestId, Esp3dMessageType::core);
}

bool Esp3DCommands::dispatch(const char* sbuf, Esp3dClient target,
                             esp3d_request_t requestId, Esp3dMessageType type,
                             Esp3dClient origin,
                             Esp3dAuthenticationLevel authentication_level) {
  Esp3dMessage* newMsgPtr = Esp3DClient::newMsg(origin, target);
  if (newMsgPtr) {
    newMsgPtr->requestId = requestId;
    newMsgPtr->type = type;
    return dispatch(newMsgPtr, sbuf);
  }
  esp3d_log_e("no newMsgPtr");
  return false;
}

bool Esp3DCommands::dispatch(Esp3dMessage* msg, const char* sbuf) {
  return dispatch(msg, (uint8_t*)sbuf, strlen(sbuf));
}

bool Esp3DCommands::dispatch(Esp3dMessage* msg, uint8_t* sbuf, size_t len) {
  if (!msg) {
    esp3d_log_e("no msg");
    return false;
  }
  if (!Esp3DClient::setDataContent(msg, sbuf, len)) {
    esp3d_log_e("Out of memory");
    Esp3DClient::deleteMsg(msg);
    return false;
  }
  return dispatch(msg);
}

bool Esp3DCommands::dispatch(Esp3dMessage* msg) {
  bool sendOk = true;
  esp3d_log("Dispatch message origin %d to client %d , size: %d,  type: %d",
            msg->origin, msg->target, msg->size, msg->type);
  if (!msg) {
    esp3d_log_e("no msg");
    return false;
  }
  // currently only echo back no test done on success
  // TODO check add is successful
  switch (msg->target) {
#if ESP3D_HTTP_FEATURE
    case Esp3dClient::webui:
      if (esp3dHttpService.started()) {
        esp3dHttpService.process(msg);
      } else {
        sendOk = false;
        esp3d_log_w("esp3dHttpService not started for message size  %d",
                    msg->size);
      }
      break;
    case Esp3dClient::webui_websocket:
      if (esp3dWsWebUiService.started()) {
        esp3dWsWebUiService.process(msg);
      } else {
        sendOk = false;
        esp3d_log_w("esp3dWsWebUiService not started for message size  %d",
                    msg->size);
      }
      break;
#if ESP3D_WS_SERVICE_FEATURE
    case Esp3dClient::websocket:
      if (esp3dWsDataService.started()) {
        esp3dWsDataService.process(msg);
      } else {
        sendOk = false;
        esp3d_log_w("esp3dWsDataService not started for message size  %d",
                    msg->size);
      }
      break;
#endif  // ESP3D_WS_SERVICE_FEATURE
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_TELNET_FEATURE
    case Esp3dClient::telnet:
      if (esp3dSocketServer.started()) {
        esp3dSocketServer.process(msg);
      } else {
        sendOk = false;
        esp3d_log_w("esp3dSocketServer not started for message size  %d",
                    msg->size);
      }
      break;
#endif  // ESP3D_TELNET_FEATURE

    case Esp3dClient::serial:
      esp3d_log("Serial client got message");
      if (serialClient.started()) {
        serialClient.process(msg);
      } else {
        sendOk = false;
        esp3d_log_w("serialClient not started for message size  %d", msg->size);
      }
      break;
#if ESP3D_USB_SERIAL_FEATURE
    case Esp3dClient::usb_serial:
      esp3d_log("USB Serial client got message");
      if (usbSerialClient.started()) {
        esp3d_log("USB Serial process message");
        usbSerialClient.process(msg);
      } else {
        sendOk = false;
        esp3d_log_e("usbSerialClient not started for message size  %d",
                    msg->size);
      }
      break;
#endif  // #if ESP3D_USB_SERIAL_FEATURE

    case Esp3dClient::all_clients:
      // msg need to be duplicate for each target
      // Do not broadcast to printer output
      // printer may receive unwhished messages
      /* //Esp3dClient::serial
       if (msg->origin!=Esp3dClient::serial) {
           if (msg->target==Esp3dClient::all_clients) {
               //become the reference message
               msg->target=Esp3dClient::serial;
           } else {
               //duplicate message because current is  already pending
               Esp3dMessage * copy_msg = Esp3DClient::copyMsg(*msg);
               if (copy_msg) {
                   copy_msg->target = Esp3dClient::serial;
                   dispatch(copy_msg);
               } else {
                   esp3d_log_e("Cannot duplicate message for Serial");
               }
           }
       }*/
      /*
      #if ESP3D_USB_SERIAL_FEATURE
      //Esp3dClient::usb_serial
      if (msg->origin!=Esp3dClient::usb_serial) {
          if (msg->target==Esp3dClient::all_clients) {
              //become the reference message
              msg->target=Esp3dClient::usb_serial;
          } else {
              //duplicate message because current is  already pending
              Esp3dMessage * copy_msg = Esp3DClient::copyMsg(*msg);
              if (copy_msg) {
                  copy_msg->target = Esp3dClient::usb_serial;
                  dispatch(copy_msg);
              } else {
                  esp3d_log_e("Cannot duplicate message for USB Serial");
              }
          }
      }
      #endif //#if ESP3D_USB_SERIAL_FEATURE*/
#if ESP3D_HTTP_FEATURE
      // Esp3dClient::webui_websocket
      if (msg->origin != Esp3dClient::webui_websocket) {
        if (msg->target == Esp3dClient::all_clients) {
          // become the reference message
          msg->target = Esp3dClient::webui_websocket;
        } else {
          // duplicate message because current is  already pending
          Esp3dMessage* copy_msg = Esp3DClient::copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = Esp3dClient::webui_websocket;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for Websocket");
          }
        }
      }
#if ESP3D_WS_SERVICE_FEATURE
      // Esp3dClient::websocket
      if (msg->origin != Esp3dClient::websocket) {
        msg->requestId.id = 0;
        if (msg->target == Esp3dClient::all_clients) {
          // become the reference message
          msg->target = Esp3dClient::websocket;
        } else {
          // duplicate message because current is  already pending
          Esp3dMessage* copy_msg = Esp3DClient::copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = Esp3dClient::websocket;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for Websocket");
          }
        }
      }
#endif  // ESP3D_WS_SERVICE_FEATURE
#endif  // #if ESP3D_HTTP_FEATURE
#if ESP3D_TELNET_FEATURE
      // Esp3dClient::telnet
      if (msg->origin != Esp3dClient::telnet) {
        msg->requestId.id = 0;
        if (msg->target == Esp3dClient::all_clients) {
          // become the reference message
          msg->target = Esp3dClient::telnet;
        } else {
          // duplicate message because current is  already pending
          Esp3dMessage* copy_msg = Esp3DClient::copyMsg(*msg);
          if (copy_msg) {
            copy_msg->target = Esp3dClient::telnet;
            dispatch(copy_msg);
          } else {
            esp3d_log_e("Cannot duplicate message for Websocket");
          }
        }
      }
#endif  // ESP3D_TELNET_FEATURE
        // Send pending if any or cancel message is no client did handle it
      if (msg->target == Esp3dClient::all_clients) {
        sendOk = false;
      } else {
        return dispatch(msg);
      }
      break;
    default:
      esp3d_log_e("No valid target specified %d", msg->target);
      sendOk = false;
  }
  // clear message
  if (!sendOk) {
    esp3d_log_w("Send msg failed");
    Esp3DClient::deleteMsg(msg);
  }
  return sendOk;
}

bool Esp3DCommands::hasTag(Esp3dMessage* msg, uint start, const char* label) {
  if (!msg) {
    esp3d_log_e("no msg for tag %s", label);
    return false;
  }
  std::string lbl = label;
  // esp3d_log("checking message for tag %s", label);
  uint lenLabel = strlen(label);
  lbl += "=";
  lbl = get_param(msg, start, lbl.c_str());
  if (lbl.length() != 0) {
    // esp3d_log("Label is used with parameter %s", lbl.c_str());
    // make result uppercase
    esp3d_strings::str_toUpperCase(&lbl);
    return (lbl == "YES" || lbl == "1" || lbl == "TRUE");
  }
  bool prevCharIsEscaped = false;
  bool prevCharIsspace = true;
  // esp3d_log("Checking  label as tag");
  for (uint i = start; i < msg->size; i++) {
    char c = char(msg->data[i]);
    // esp3d_log("%c", c);
    if (c == label[0] && prevCharIsspace) {
      uint p = 0;
      while (i < msg->size && p < lenLabel && c == label[p]) {
        i++;
        p++;
        if (i < msg->size) {
          c = char(msg->data[i]);
          // esp3d_log("%c vs %c", c, char(msg->data[i]));
        }
      }
      if (p == lenLabel) {
        // end of params
        if (i == msg->size || std::isspace(c)) {
          // esp3d_log("label %s found", label);
          return true;
        }
      }
      if (std::isspace(c) && !prevCharIsEscaped) {
        prevCharIsspace = true;
      }
      if (c == '\\') {
        prevCharIsEscaped = true;
      } else {
        prevCharIsEscaped = false;
      }
    }
  }
  // esp3d_log("label %s not found", label);
  return false;
}

const char* Esp3DCommands::get_param(Esp3dMessage* msg, uint start,
                                     const char* label) {
  if (!msg) {
    return "";
  }
  return get_param((const char*)msg->data, msg->size, start, label);
}

const char* Esp3DCommands::get_param(const char* data, uint size, uint start,
                                     const char* label) {
  int startPos = -1;
  uint lenLabel = strlen(label);
  static std::string value;
  bool prevCharIsEscaped = false;
  bool prevCharIsspace = true;
  value.clear();
  uint startp = start;
  while (char(data[startp]) == ' ' && startp < size) {
    startp++;
  }
  for (uint i = startp; i < size; i++) {
    char c = char(data[i]);
    if (c == label[0] && startPos == -1 && prevCharIsspace) {
      uint p = 0;
      while (i < size && p < lenLabel && c == label[p]) {
        i++;
        p++;
        if (i < size) {
          c = char(data[i]);
        }
      }
      if (p == lenLabel) {
        startPos = i;
      }
    }
    if (std::isspace(c) && !prevCharIsEscaped) {
      prevCharIsspace = true;
    }
    if (startPos > -1 && i < size) {
      if (c == '\\') {
        prevCharIsEscaped = true;
      }
      if (std::isspace(c) && !prevCharIsEscaped) {
        return value.c_str();
      }

      if (c != '\\') {
        value.append(1, c);
        prevCharIsEscaped = false;
      }
    }
  }
  return value.c_str();
}

const char* Esp3DCommands::get_clean_param(Esp3dMessage* msg, uint start) {
  if (!msg) {
    return "";
  }
  static std::string value;
  bool prevCharIsEscaped = false;
  uint startp = start;
  while (char(msg->data[startp]) == ' ' && startp < msg->size) {
    startp++;
  }
  value.clear();
  for (uint i = startp; i < msg->size; i++) {
    char c = char(msg->data[i]);
    if (c == '\\') {
      prevCharIsEscaped = true;
    }
    if (std::isspace(c) && !prevCharIsEscaped) {
      // esp3d_log("testing *%s*", value.c_str());
      if (value == "json" ||
          esp3d_strings::startsWith(value.c_str(), "json=") ||
          esp3d_strings::startsWith(value.c_str(), "pwd=")) {
        value.clear();
      } else {
        return value.c_str();
      }
    }
    if (c != '\\') {
      if ((std::isspace(c) && prevCharIsEscaped) || !std::isspace(c)) {
        value.append(1, c);
      }
      prevCharIsEscaped = false;
    }
  }
  // for empty value
  if (value == "json" || esp3d_strings::startsWith(value.c_str(), "json=") ||
      esp3d_strings::startsWith(value.c_str(), "pwd=")) {
    value.clear();
  }
  return value.c_str();
}

bool Esp3DCommands::has_param(Esp3dMessage* msg, uint start) {
  return strlen(get_clean_param(msg, start)) != 0;
}

void Esp3DCommands::execute_internal_command(int cmd, int cmd_params_pos,
                                             Esp3dMessage* msg) {
  // execute commands
  if (!msg) {
    return;
  }
#if ESP3D_AUTHENTICATION_FEATURE
  std::string pwd = get_param(msg, cmd_params_pos, "pwd=");
  if (!pwd.empty()) {  // adjust authentication level according
    Esp3dAuthenticationLevel lvl =
        esp3dAuthenthicationService.getAuthenticatedLevel(pwd.c_str());
    if (msg->authentication_level != lvl) {
      esp3d_log("Authentication level change from %d to %d",
                msg->authentication_level, lvl);
      msg->authentication_level = lvl;
      if (!esp3dAuthenthicationService.updateRecord(msg->requestId.id,
                                                    msg->origin, lvl)) {
        esp3d_log_e("Did not found the corresponding session");
      }
    }
  }
#if ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
  if (msg->origin == Esp3dClient::serial) {
    msg->authentication_level = Esp3dAuthenticationLevel::admin;
  }
#endif  // ESP3D_DISABLE_SERIAL_AUTHENTICATION_FEATURE
#endif  // ESP3D_AUTHENTICATION_FEATURE

  switch (cmd) {
    case 0:
      ESP0(cmd_params_pos, msg);
      break;
#if ESP3D_WIFI_FEATURE
    case 100:
      ESP100(cmd_params_pos, msg);
      break;
    case 101:
      ESP101(cmd_params_pos, msg);
      break;
    case 102:
      ESP102(cmd_params_pos, msg);
      break;
    case 103:
      ESP103(cmd_params_pos, msg);
      break;
    case 104:
      ESP104(cmd_params_pos, msg);
      break;
    case 105:
      ESP105(cmd_params_pos, msg);
      break;
    case 106:
      ESP106(cmd_params_pos, msg);
      break;
    case 107:
      ESP107(cmd_params_pos, msg);
      break;
    case 108:
      ESP108(cmd_params_pos, msg);
      break;
#endif  // ESP3D_WIFI_FEATURE
    case 110:
      ESP110(cmd_params_pos, msg);
      break;
#if ESP3D_WIFI_FEATURE
    case 111:
      ESP111(cmd_params_pos, msg);
      break;
#endif  // ESP3D_WIFI_FEATURE
    case 112:
      ESP112(cmd_params_pos, msg);
      break;
    case 114:
      ESP114(cmd_params_pos, msg);
      break;
    case 115:
      ESP115(cmd_params_pos, msg);
      break;
#if ESP3D_HTTP_FEATURE
    case 120:
      ESP120(cmd_params_pos, msg);
      break;
    case 121:
      ESP121(cmd_params_pos, msg);
      break;
#endif  // ESP3D_HTTP_FEATURE

#if ESP3D_TELNET_FEATURE
    case 130:
      ESP130(cmd_params_pos, msg);
      break;
    case 131:
      ESP131(cmd_params_pos, msg);
      break;
#endif  // ESP3D_TELNET_FEATURE
#if ESP3D_HTTP_FEATURE
#if ESP3D_WS_SERVICE_FEATURE
    case 160:
      ESP160(cmd_params_pos, msg);
      break;
#endif  // ESP3D_WS_SERVICE_FEATURE
#endif  // ESP3D_HTTP_FEATURE
#if ESP3D_SD_CARD_FEATURE
    case 200:
      ESP200(cmd_params_pos, msg);
      break;
#if ESP3D_SD_FEATURE_IS_SPI
    case 202:
      ESP202(cmd_params_pos, msg);
      break;
#endif  // ESP3D_SD_FEATURE_IS_SPI
#endif  // ESP3D_SD_CARD_FEATURE

    case 400:
      ESP400(cmd_params_pos, msg);
      break;
    case 401:
      ESP401(cmd_params_pos, msg);
      break;
#if ESP3D_SD_CARD_FEATURE
#if ESP3D_UPDATE_FEATURE
    case 402:
      ESP402(cmd_params_pos, msg);
      break;
#endif  // ESP3D_UPDATE_FEATURE
#endif  // ESP3D_SD_CARD_FEATURE
#if ESP3D_WIFI_FEATURE
    case 410:
      ESP410(cmd_params_pos, msg);
      break;
#endif  // ESP3D_WIFI_FEATURE
    case 420:
      ESP420(cmd_params_pos, msg);
      break;
    case 444:
      ESP444(cmd_params_pos, msg);
      break;
#if ESP3D_MDNS_FEATURE
    case 450:
      ESP450(cmd_params_pos, msg);
      break;
#endif  // ESP3D_MDNS_FEATURE

#if ESP3D_AUTHENTICATION_FEATURE
    case 500:
      ESP500(cmd_params_pos, msg);
      break;
    case 510:
      ESP510(cmd_params_pos, msg);
      break;
    case 550:
      ESP550(cmd_params_pos, msg);
      break;
    case 555:
      ESP555(cmd_params_pos, msg);
      break;
#endif  // ESP3D_AUTHENTICATION_FEATURE
#if ESP3D_NOTIFICATIONS_FEATURE
    case 600:
      ESP600(cmd_params_pos, msg);
      break;
    case 610:
      ESP610(cmd_params_pos, msg);
      break;
#endif  // ESP3D_NOTIFICATIONS_FEATURE
#if ESP3D_GCODE_HOST_FEATURE
    case 700:
      ESP700(cmd_params_pos, msg);
      break;
    case 701:
      ESP701(cmd_params_pos, msg);
      break;
#endif  // ESP3D_GCODE_HOST_FEATURE
    case 710:
      ESP710(cmd_params_pos, msg);
      break;
    case 720:
      ESP720(cmd_params_pos, msg);
      break;
    case 730:
      ESP730(cmd_params_pos, msg);
      break;
#if ESP3D_SD_CARD_FEATURE
    case 740:
      ESP740(cmd_params_pos, msg);
      break;
    case 750:
      ESP750(cmd_params_pos, msg);
      break;
#endif  // ESP3D_SD_CARD_FEATURE
    case 780:
      ESP780(cmd_params_pos, msg);
      break;
    case 790:
      ESP790(cmd_params_pos, msg);
      break;
    case 900:
      ESP900(cmd_params_pos, msg);
      break;
    case 800:
      ESP800(cmd_params_pos, msg);
      break;
    case 901:
      ESP901(cmd_params_pos, msg);
      break;
#if ESP3D_USB_SERIAL_FEATURE

    case 902:
      ESP902(cmd_params_pos, msg);
      break;
    case 950:
      ESP950(cmd_params_pos, msg);
      break;
#endif  // #if ESP3D_USB_SERIAL_FEATURE
    default:
      msg->target = msg->origin;
      if (hasTag(msg, cmd_params_pos, "json")) {
        if (!dispatch(msg,
                      "{\"cmd\":\"0\",\"status\":\"error\",\"data\":\"Invalid "
                      "Command\"}")) {
          esp3d_log_e("Out of memory");
        }
      } else {
        if (!dispatch(msg, "Invalid Command\n")) {
          esp3d_log_e("Out of memory");
        }
      }
  }
}

void Esp3DCommands::flush() { serialClient.flush(); }

bool isRealTimeCommand(char* cmd, size_t len) { return false; }

bool Esp3DCommands::formatCommand(char* cmd, size_t len) {
  if (isRealTimeCommand(cmd, len)) {
    // TODO
    return true;
  }
  uint sizestr = strlen(cmd);
  if (len > sizestr + 2) {
    cmd[sizestr] = '\n';
    cmd[sizestr + 1] = 0x0;
    return true;
  }
  return false;
}

Esp3dClient Esp3DCommands::getOutputClient(bool fromSettings) {
  if (fromSettings) {
    _output_client = Esp3dClient::serial;
#if ESP3D_USB_SERIAL_FEATURE
    uint8_t value = esp3dTFTsettings.readByte(esp3d_output_client);
    switch ((Esp3dClient)value) {
      case Esp3dClient::serial:
        _output_client = Esp3dClient::serial;
        break;
      case Esp3dClient::usb_serial:
        _output_client = Esp3dClient::usb_serial;
        break;
      default:
        _output_client = Esp3dClient::serial;
        break;
    }
#endif  // #if ESP3D_USB_SERIAL_FEATURE
  }
  esp3d_log("Output client is %d", _output_client);
  return _output_client;
}