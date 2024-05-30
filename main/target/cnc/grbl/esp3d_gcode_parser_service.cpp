/*
  esp3d_gcode_parser_service
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

#include "esp3d_gcode_parser_service.h"

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_string.h"
#include "esp3d_values.h"

ESP3DGCodeParserService esp3dGcodeParser;

const char* emmergencyGcodeCommand[] = {"M112", "M410", "M999"};
const char* emmergencyESP3DCommand[] = {"[ESP701]"};
const char* pollingCommands[] = {
    "?",             // Status
    "$G",            // GCode parser state
    "$#",            // GCode Parameters
    "[ESP701]json",  // streaming status
};

uint64_t pollingCommandsLastUpdate[] = {
    0,  // Status
    0,  // GCode parser state
    0,  // GCode Parameters
    0,  // streaming status
};

const char* screenCommands[] = {"M117",  // TFT screen output
                                ""};
const char* no_ack_commands[] = {  // Commands that do not need an ack
    ""};

const char* fwCommands[] = {"M110 N0",  // reset stream numbering
                            ""};
uint64_t ESP3DGCodeParserService::getPollingCommandsLastRun(uint8_t index) {
  if (index < ESP3D_POLLING_COMMANDS_COUNT) {
    return pollingCommandsLastUpdate[index];
  }
  esp3d_log_e("Error polling command index out of range");
  return 0;
}
bool ESP3DGCodeParserService::setPollingCommandsLastRun(uint8_t index,
                                                        uint64_t value) {
  if (index < ESP3D_POLLING_COMMANDS_COUNT) {
    pollingCommandsLastUpdate[index] = value;
    return true;
  } else {
    esp3d_log_e("Error polling command index out of range");
    return false;
  }
}
const char* ESP3DGCodeParserService::getFwCommandString(FW_GCodeCommand cmd) {
  return fwCommands[(uint8_t)cmd];
}
const char** ESP3DGCodeParserService::getPollingCommands() {
  return pollingCommands;
}

bool ESP3DGCodeParserService::forwardToScreen(const char* command) {
  for (uint8_t i = 0; strlen(screenCommands[i]) != 0; i++) {
    if (strncmp(command, screenCommands[i], strlen(screenCommands[i])) == 0) {
      esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                      &command[strlen(screenCommands[i])]);
      // TODO: Forward to Notification / WebUI ?
      // ESP3DNotificationsService::sendMSG(const char* title, const char*
      // message)
#if ESP3D_HTTP_FEATURE
      // esp3dWsWebUiService.pushNotification(
      // &command[strlen(screenCommands[i])]);
#endif  // ESP3D_HTTP_FEATURE
      return true;
    }
  }
  return false;
}

ESP3DGCodeParserService::ESP3DGCodeParserService() {
  _isMultiLineReportOnGoing = false;
}

ESP3DGCodeParserService::~ESP3DGCodeParserService() {
  _isMultiLineReportOnGoing = false;
}

// check if command generate an ack
// by default almost all commands need an ack
bool ESP3DGCodeParserService::hasAck(const char* command) {
  for (uint8_t i = 0; strlen(no_ack_commands[i]) != 0; i++) {
    if (strncmp(command, no_ack_commands[i], strlen(no_ack_commands[i])) == 0) {
      return false;
    }
  }
  return true;
}

// TODO: implement multi line report detection
bool ESP3DGCodeParserService::hasMultiLineReport(const char* data) {
  return false;
}

bool ESP3DGCodeParserService::processCommand(const char* data) {
  esp3d_log("processing Command %s", data);
  // if (data != nullptr && strlen(data) > 0) {
  //   // is temperature
  //   if (strstr(data, "T:") != nullptr) {
  //     // ok T:25.00 /120.00 B:25.00 /0.00 @:127 B@:0
  //     // T:25.00 /0.00 B:25.00 /50.00 T0:25.00 /0.00 T1:105.00 /0.00 @:0
  //     B@:127 char* ptrt = strstr(data, "T:"); char* ptrt0 = strstr(data,
  //     "T0:"); char* ptrt1 = strstr(data, "T1:"); char* ptrb = strstr(data,
  //     "B:"); if (ptrt0 && ptrt1) {  // dual extruder
  //       esp3d_log("Temperature dual extruders");
  //       ptrt0 += 3;
  //       ptrt1 += 3;
  //       // Extruder 0 current temperature
  //       char* ptre = strstr(ptrt0, " /");
  //       if (!ptre) {
  //         esp3d_log_e("Error parsing temperature T0 value");
  //         return false;
  //       }
  //       ptre[0] = '\0';
  //       // Extruder 0 target temperature
  //       char* ptrtt = ptre + 2;
  //       ptre = strstr(ptrtt, " ");
  //       if (!ptre) {
  //         esp3d_log_e("Error parsing temperature T0 target");
  //         return false;
  //       }
  //       ptre[0] = '\0';
  //       // Dispatch values
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_0_temperature,
  //                                       ptrt0);
  //       esp3dTftValues.set_string_value(
  //           ESP3DValuesIndex::ext_0_target_temperature, ptrtt);
  //       esp3d_log("T0: %s / %s", ptrt0, ptrtt);

  //       // Extruder 1 current temperature
  //       ptre = strstr(ptrt1, " /");
  //       if (!ptre) {
  //         esp3d_log_e("Error parsing temperature T1 value");
  //         esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_1_temperature,
  //                                         "#");
  //         esp3dTftValues.set_string_value(
  //             ESP3DValuesIndex::ext_1_target_temperature, "#");
  //         return false;
  //       }
  //       ptre[0] = '\0';
  //       // Extruder 1 target temperature
  //       ptrtt = ptre + 2;
  //       ptre = strstr(ptrtt, " ");
  //       if (!ptre) {
  //         esp3d_log_e("Error parsing temperature T1 target");
  //         esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_1_temperature,
  //                                         "#");
  //         esp3dTftValues.set_string_value(
  //             ESP3DValuesIndex::ext_1_target_temperature, "#");
  //         return false;
  //       }
  //       ptre[0] = '\0';
  //       // Dispatch values
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_1_temperature,
  //                                       ptrt1);
  //       esp3dTftValues.set_string_value(
  //           ESP3DValuesIndex::ext_1_target_temperature, ptrtt);
  //       esp3d_log("T1: %s / %s", ptrt1, ptrtt);
  //     } else {  // single extruder
  //       esp3d_log("Temperature single extruder");
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_1_temperature,
  //                                       "#");
  //       esp3dTftValues.set_string_value(
  //           ESP3DValuesIndex::ext_1_target_temperature, "#");
  //       ptrt += 2;
  //       // Extruder 0 current temperature
  //       char* ptre = strstr(ptrt, " /");
  //       if (!ptre) {
  //         esp3d_log_e("Error parsing temperature T0 value");
  //         return false;
  //       }
  //       ptre[0] = '\0';
  //       // Extruder 0 target temperature
  //       char* ptrtt = ptre + 2;
  //       ptre = strstr(ptrtt, " ");
  //       if (!ptre) {
  //         esp3d_log_e("Error parsing temperature T0 target");
  //         return false;
  //       }
  //       ptre[0] = '\0';
  //       // Dispatch values
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_0_temperature,
  //                                       ptrt);
  //       esp3dTftValues.set_string_value(
  //           ESP3DValuesIndex::ext_0_target_temperature, ptrtt);

  //       esp3d_log("T: %s / %s", ptrt, ptrtt);
  //     }
  //     if (ptrb) {  // bed
  //       esp3d_log("Temperature bed");
  //       ptrb += 2;
  //       // Bed current temperature
  //       char* ptre = strstr(ptrb, " /");
  //       if (!ptre) {
  //         esp3d_log_e("Error parsing temperature bed value");
  //         return false;
  //       }
  //       ptre[0] = '\0';
  //       // Bed target temperature
  //       char* ptrtt = ptre + 2;
  //       ptre = strstr(ptrtt, " ");
  //       if (!ptre) {
  //         esp3d_log_e("Error parsing temperature Bed target");
  //         return false;
  //       }
  //       ptre[0] = '\0';
  //       // Dispatch values
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::bed_temperature,
  //                                       ptrb);
  //       esp3dTftValues.set_string_value(
  //           ESP3DValuesIndex::bed_target_temperature, ptrtt);
  //       esp3d_log("Bed: %s / %s", ptrb, ptrtt);
  //     } else {
  //       esp3d_log("No Temperature bed");
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::bed_temperature,
  //       "#"); esp3dTftValues.set_string_value(
  //           ESP3DValuesIndex::bed_target_temperature, "#");
  //     }
  //     setPollingCommandsLastRun(
  //         ESP3D_POLLING_COMMANDS_INDEX_TEMPERATURE_TEMPERATURE,
  //         esp3d_hal::millis());
  //     return true;
  //     // is position but not bed leveling
  //   } else if (strstr(data, "X:") != nullptr &&
  //              strstr(data, "Bed X:") == nullptr) {
  //     // X:0.00 Y:0.00 Z:0.00 E:0.00 Count X:0 Y:0 Z:0
  //     esp3d_log("Positions");
  //     char* ptrx = strstr(data, "X:");
  //     char* ptry = strstr(data, "Y:");
  //     char* ptrz = strstr(data, "Z:");
  //     char* ptre = strstr(data, "E:");

  //     if (ptrx && ptry && ptrz && ptre) {
  //       ptrx += 2;
  //       ptry[0] = '\0';
  //       ptry += 2;
  //       ptrz[0] = '\0';
  //       ptrz += 2;
  //       if (ptre) {
  //         ptre[0] = '\0';
  //       }
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::position_x, ptrx);
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::position_y, ptry);
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::position_z, ptrz);
  //       setPollingCommandsLastRun(
  //           ESP3D_POLLING_COMMANDS_INDEX_TEMPERATURE_POSITION,
  //           esp3d_hal::millis());
  //       return true;
  //     } else {
  //       esp3d_log_e("Error parsing positions");
  //     }
  //   } else if (strstr(data, "FR:") != nullptr) {
  //     char* ptrfr = strstr(data, "FR:");
  //     char* ptrpc = strstr(data, "%");
  //     if (ptrfr && ptrpc) {
  //       ptrfr += 3;
  //       ptrpc[0] = '\0';
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::speed, ptrfr);
  //       setPollingCommandsLastRun(
  //           ESP3D_POLLING_COMMANDS_INDEX_TEMPERATURE_SPEED,
  //           esp3d_hal::millis());
  //       return true;
  //     } else {
  //       esp3d_log_e("Error parsing progress");
  //     }
  //     // is fan speed ?
  //   } else if (strstr(data, "M106") != nullptr ||
  //              strstr(data, "M107") != nullptr) {
  //     char* ptr106 = strstr(data, "M106");
  //     char* ptr107 = strstr(data, "M107");
  //     if (ptr106) {
  //       ptr106 += 4;
  //       char* ptrS = strstr(ptr106, "S");
  //       if (!ptrS) {
  //         esp3d_log_e("Error parsing fan speed");
  //         return false;
  //       }

  //       ptrS++;
  //       // get fan speed
  //       static std::string fanSpeed = "";
  //       fanSpeed = "";

  //       for (uint8_t i = 0; i < 3; i++) {
  //         if (ptrS[i] >= '0' && ptrS[i] <= '9') {
  //           fanSpeed += ptrS[i];
  //         } else {
  //           break;
  //         }
  //       }
  //       // is there an index ?
  //       esp3d_log("Check index in %s", ptr106);
  //       uint8_t index = 0;
  //       char* ptrI = strstr(ptr106, "P");
  //       if (ptrI) {
  //         esp3d_log("Index found %s", ptrI);
  //         if (ptrI[1] == '1') {
  //           index = 1;
  //         }
  //       }
  //       // conversion 0~255 to 0~100
  //       double fspeed = (std::stod(fanSpeed.c_str()) * 100) / 255;
  //       // limit to 100%
  //       if (fspeed > 100) {
  //         fspeed = 100;
  //       }

  //       fanSpeed = esp3d_string::set_precision(std::to_string(fspeed), 0);
  //       // set fan speed according index
  //       esp3d_log("Fan speed 106, index: %d, %s", index, fanSpeed.c_str());
  //       if (index == 0) {
  //         esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_0_fan,
  //                                         fanSpeed.c_str());
  //       } else {
  //         esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_1_fan,
  //                                         fanSpeed.c_str());
  //       }
  //       return true;
  //     } else if (ptr107) {
  //       ptr107 += 4;
  //       char* ptrI = strstr(ptr107, "P");
  //       uint8_t index = 0;
  //       // is there an index ?
  //       if (ptrI) {
  //         if (ptrI[1] == '1') {
  //           index = 1;
  //         }
  //       }
  //       esp3d_log("Fan speed 107, index: %d", index);
  //       // set fan speed to 0 according index
  //       if (index == 0) {
  //         esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_0_fan, "0");
  //       } else {
  //         esp3dTftValues.set_string_value(ESP3DValuesIndex::ext_1_fan, "0");
  //       }
  //       return true;
  //     } else {
  //       esp3d_log_e("Error parsing fan speed");
  //     }
  //     // G29 Auto Bed Leveling
  //   } else if (strstr(data, "G29 Auto Bed Leveling") != nullptr ||
  //              strstr(data, "Bed X:") != nullptr ||
  //              strstr(data, "Bilinear Leveling Grid:") != nullptr) {
  //     static bool isLeveling = false;
  //     if (strstr(data, "G29 Auto Bed Leveling") != nullptr) {
  //       isLeveling = true;
  //       // Send start of leveling
  //       esp3dTftValues.set_string_value(ESP3DValuesIndex::bed_leveling,
  //       "Start",
  //                                       ESP3DValuesCbAction::Add);
  //     } else if (isLeveling) {
  //       if (strstr(data, "Bilinear Leveling Grid:") != nullptr) {
  //         isLeveling = false;
  //         // Send end of leveling
  //         esp3dTftValues.set_string_value(ESP3DValuesIndex::bed_leveling,
  //         "End",
  //                                         ESP3DValuesCbAction::Delete);
  //       } else {
  //         // Send leveling data
  //         esp3dTftValues.set_string_value(ESP3DValuesIndex::bed_leveling,
  //         data,
  //                                         ESP3DValuesCbAction::Update);
  //       }
  //     }
  //   }
  // }
  return false;
}

ESP3DDataType ESP3DGCodeParserService::getType(const char* data) {
  if (data == nullptr) {
    return ESP3DDataType::empty_line;
  }
  char* ptr = (char*)data;

  // remove leading spaces and tabs
  for (uint8_t i = 0; i < strlen(data); i++) {
    if (data[i] == ' ' || data[i] == '\t') {
      ptr++;
    } else {
      break;
    }
  }

  // is empty line ?
  if (ptr[0] == '\n' || ptr[0] == '\r') {
    return ESP3DDataType::empty_line;
  }

  // is it ack ?
  if ((ptr[0] == 'o' && ptr[1] == 'k') &&
      (ptr[2] == '\n' || ptr[2] == '\r' || ptr[2] == ' ' || ptr[2] == 0x0)) {
    return ESP3DDataType::ack;
  }

  // is it comment ?
  if (ptr[0] == ';' || ptr[0] == '#') {
    return ESP3DDataType::comment;
  }

  // is gcode ?
  if (ptr[0] == 'M' || ptr[0] == 'G' || ptr[0] == 'T') {
    char* ptr2 = ptr;
    int hasNumber = 0;
    for (uint8_t i = 0; i < strlen(ptr2); i++) {
      if (ptr2[i] >= '0' && ptr2[i] <= '9') {
        hasNumber++;
      } else if ((ptr2[i] == ' ' || ptr2[i] == '\t' || ptr2[i] == '\n' ||
                  ptr2[i] == '\r') &&
                 hasNumber > 0 && hasNumber < 5) {
        for (uint8_t i = 0; i < sizeof(emmergencyGcodeCommand) / sizeof(char*);
             i++) {
          if (strstr(ptr, emmergencyGcodeCommand[i]) == ptr) {
            return ESP3DDataType::emergency_command;
          }
        }
        return ESP3DDataType::gcode;
      } else {
        // not a gcode
        break;
      }
    }
  }

  // is it resend ?
  // Resend: 2
  if (strstr(ptr, "Resend: ") == ptr) {
    _lineResend = atoi(ptr + 8);
    return ESP3DDataType::resend;
  }

  // is it error ?
  // Error:Line Number is not Last Line Number + 1 Last Line : 1
  if (strstr(ptr, "Error:") == ptr) {
    _lastError = ptr + 6;
    return ESP3DDataType::error;
  }

  // is it status ?
  if (strstr(ptr, "busy") == ptr || strstr(ptr, "processing") == ptr ||
      strstr(ptr, "heating") == ptr || strstr(ptr, "echo:busy") == ptr ||
      strstr(ptr, "echo:processing") == ptr ||
      strstr(ptr, "echo:heating") == ptr) {
    esp3d_log("Status: %s", esp3d_string::str_trim(ptr));
    return ESP3DDataType::status;
  }
  /*
    "Not SD printing"
    "Done printing file"
    "SD printing byte"
    "echo: M73 Progress:"
    "echo:Print time:"
    "echo:E"
    "Current file:"
    "FR:xxx%"
    "Cap:"
    "FIRMWARE_NAME:"
    "ok T:25.00 /120.00 B:25.00 /0.00 @:127 B@:0"
    "T:25.00 /0.00 B:25.00 /50.00 T0:25.00 /0.00 T1:25.00 /0.00 @:0 B@:127
    @0:0
    @1:0"
    "ok T0:23.00 /0.00 B:22.62 /0.00 T0:23.00 /0.00 T1:23.08 /0.00 @:0 B@:0
    @0:0
    @1:0"
    "X:0.00 Y:0.00 Z:0.00 E:0.00 Count X:0 Y:0 Z:0"
    */

  // is it response ?
  if (strstr(ptr, "echo:") == ptr || strstr(ptr, "ok T:") == ptr ||
      strstr(ptr, "T:") == ptr || strstr(ptr, "X:") == ptr ||
      strstr(ptr, "Not SD printing") == ptr ||
      strstr(ptr, "Done printing file") == ptr ||
      strstr(ptr, "SD printing byte") == ptr ||
      strstr(ptr, "Current file:") == ptr || strstr(ptr, "FR:") == ptr ||
      strstr(ptr, "Cap:") == ptr || strstr(ptr, "FIRMWARE_NAME:") == ptr ||
      strstr(ptr, "ok T0:") == ptr || strstr(ptr, "T0:") == ptr) {
    return ESP3DDataType::response;
  }

  // is [ESPxxx] command ?
  if (strstr(ptr, "[ESP") == ptr) {
    if (strlen(ptr) >= 5 &&
        (ptr[4] == ']' || (ptr[4] >= '0' && ptr[4] <= '9'))) {
      for (uint8_t i = 0; i < sizeof(emmergencyESP3DCommand) / sizeof(char*);
           i++) {
        if (strstr(ptr, emmergencyESP3DCommand[i]) == ptr) {
          return ESP3DDataType::emergency_command;
        }
      }
      return ESP3DDataType::esp_command;
    }
  }

  return ESP3DDataType::unknown;
}
