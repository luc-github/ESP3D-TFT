/*
  esp3d_gcode_host_service

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
#include "esp3d_gcode_host_service.h"

#include <stdio.h>

#include "serial/esp3d_serial_client.h"
#if ESP3D_USB_SERIAL_FEATURE
#include "usb_serial/esp3d_usb_serial_client.h"
#endif  // #if ESP3D_USB_SERIAL_FEATURE

#include "esp3d_commands.h"
#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "esp3d_settings.h"
#include "esp3d_values.h"
#include "filesystem/esp3d_flash.h"
#include "filesystem/esp3d_globalfs.h"
#if ESP3D_NOTIFICATIONS_FEATURE
#include "notifications/esp3d_notifications_service.h"
#endif  // ESP3D_NOTIFICATIONS_FEATURE

#if ESP3D_SD_CARD_FEATURE
#include "filesystem/esp3d_sd.h"
#endif  // ESP3D_SD_CARD_FEATURE

#include "esp32/rom/crc.h"
#include "esp3d_gcode_parser_service.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks_def.h"
#include "translations/esp3d_translation_service.h"

// Macro/command behaviour:
// to be executed regardless of streaming state (ie paused)
// to be executed in same order as received
ESP3DGCodeHostService gcodeHostService;

// Macro definitions

#define RX_FLUSH_TIME_OUT 1500  // milliseconds timeout
#define MAX_COMMAND_LENGTH 255
#define ESP3D_COMMAND_TIMEOUT 10000  // milliseconds timeout
#define ESP3D_MAX_RETRY 5
#define ESP3D_REFRESH_INTERVAL 1000  // milliseconds

#define isFileStreamType(type)                      \
  ((type == ESP3DGcodeHostStreamType::fs_stream) || \
   (type == ESP3DGcodeHostStreamType::sd_stream) || \
   (type == ESP3DGcodeHostStreamType::fs_script) || \
   (type == ESP3DGcodeHostStreamType::sd_script))

#define isFileStream(stream)                                \
  ((stream->type == ESP3DGcodeHostStreamType::fs_stream) || \
   (stream->type == ESP3DGcodeHostStreamType::sd_stream) || \
   (stream->type == ESP3DGcodeHostStreamType::fs_script) || \
   (stream->type == ESP3DGcodeHostStreamType::sd_script))

#define isCommandStream(stream)                                  \
  ((stream->type == ESP3DGcodeHostStreamType::single_command) || \
   (stream->type == ESP3DGcodeHostStreamType::multiple_commands))

// main task
static void esp3d_gcode_host_task(void* pvParameter) {
  (void)pvParameter;
  gcodeHostService.updateScripts();
  esp3d_hal::wait(100);
  while (1) {
    /* Delay */
    gcodeHostService.handle(); 
    esp3d_hal::wait(10);
  }
  vTaskDelete(NULL);
}

/// @brief Ascertains the type of stream referred to by a string.
/// @param file Const Char Pointer, to an array containing a command string, or
/// a file system path.
/// @return Returns a stream type, either FS, SD, single or multiple commands.
ESP3DGcodeHostStreamType ESP3DGCodeHostService::_getStreamType(
    const char* data) {  // maybe this can also check for invalid file
                         // extensions if not done elsewhere
  esp3d_log("Get stream type for %s", data);
  if (strlen(data) == 0) {
    esp3d_log_e("Invalid data stream");
    return ESP3DGcodeHostStreamType::invalid;
  }
  if (data[0] == '/') {
    if (strstr(data, ESP3D_FLASH_FS_HEADER) == data) {
      return ESP3DGcodeHostStreamType::fs_stream;
    }
#if ESP3D_SD_CARD_FEATURE
    if (strstr(data, ESP3D_SD_FS_HEADER) == data) {
      return ESP3DGcodeHostStreamType::sd_stream;
    }
#endif  // ESP3D_SD_CARD_FEATURE

  } else {
    // FIXME:
    //  That part is problematic [ESPXXX] can contain ';' in password /
    //  parameters so we should not consider it as a multiple command
    // so for the moment multiple commands line delimited by ';' do not support
    // command with ';' in it
    if (strstr(data, "\n") != nullptr || (strstr(data, ";") != nullptr)) {
      return ESP3DGcodeHostStreamType::multiple_commands;
    } else {
      return ESP3DGcodeHostStreamType::single_command;
    }
  }
  esp3d_log_e("Invalid data stream");
  return ESP3DGcodeHostStreamType::invalid;
}

// Update scripts from settings
void ESP3DGCodeHostService::updateScripts() {
  char buffer[SIZE_OF_SCRIPT + 1];
  _stop_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_stop_script, buffer, SIZE_OF_SCRIPT);
  _resume_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_resume_script, buffer, SIZE_OF_SCRIPT);
  _pause_script = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_pause_script, buffer, SIZE_OF_SCRIPT);
}

// Create a new stream from a command msg
bool ESP3DGCodeHostService::addStream(
    const char* command, size_t length,
    ESP3DAuthenticationLevel authentication_level) {
  // the command has a 0x0 terminal so we should be able to ignore the length
  // because it is msg we should consider it as a command so we can execute it
  // first
  // so let's trim it to remove any space or \n
  std::string cmd = esp3d_string::str_trim(command);
  if (cmd.length() == 0) {
    esp3d_log_e("Empty command");
    return false;
  }
  return _add_stream(cmd.c_str(), authentication_level, true);
}

bool ESP3DGCodeHostService::hasStreamListCommand(const char* command) {
  std::string cmd = esp3d_string::str_trim(command);
  bool res = false;
  if (_streams_list_mutex) {
    if (pthread_mutex_lock(&_streams_list_mutex) == 0) {
      for (auto it = _scripts.begin(); it != _scripts.end(); ++it) {
        if (((*it)->type == ESP3DGcodeHostStreamType::single_command)) {
          std::string cmd_it = esp3d_string::str_trim((*it)->dataStream);
          if (cmd_it == cmd) {
            res = true;
            break;
          }
        }
      }
      pthread_mutex_unlock(&_streams_list_mutex);
    } else {
      esp3d_log("Failed to lock stream list mutex");
    }
  } else {
    esp3d_log("Stream list mutex not initialized");
  }
  return res;
}

// Add stream from ESP700 command
bool ESP3DGCodeHostService::addStream(const char* filename,
                                      ESP3DAuthenticationLevel auth_type,
                                      bool executeAsMacro) {
  esp3d_log("Add stream: %s", filename);
  ESP3DGcodeHostStreamType type = _getStreamType(filename);
  //  ESP700 only accepts file names, not commands
  //  as commands can be sent directly, no need to use ESP700
  if (!isFileStreamType(type)) {
    esp3d_log_e("Invalid data stream %s",
                ESP3DGcodeHostStreamTypeStr[static_cast<uint8_t>(type)]);
    return false;
  }
  return _add_stream(filename, auth_type, executeAsMacro);
}

bool ESP3DGCodeHostService::_add_stream(const char* data,
                                        ESP3DAuthenticationLevel auth_type,
                                        bool executeFirst) {
  esp3d_log("Processing stream request: %s, with authentication level=%d", data,
            static_cast<uint8_t>(auth_type));
  // Macro should be executed first like any other command
  // only file stream is executed after current stream is finished
  ESP3DGcodeHostStreamType type = _getStreamType(data);
  esp3d_log("File type: %d , %s", static_cast<uint8_t>(type),
            ESP3DGcodeHostStreamTypeStr[static_cast<uint8_t>(type)]);
  if (type == ESP3DGcodeHostStreamType::invalid) {
    esp3d_log_e("Invalid data stream");
    return false;
  }
  // any stream that is first is a macro/script
  if (executeFirst) {
    if (type == ESP3DGcodeHostStreamType::fs_stream) {
      type = ESP3DGcodeHostStreamType::fs_script;
#if ESP3D_SD_CARD_FEATURE
    } else if (type == ESP3DGcodeHostStreamType::sd_stream) {
      type = ESP3DGcodeHostStreamType::sd_script;
#endif  // ESP3D_SD_CARD_FEATURE
    }
  }
  if (_scripts.size() >= ESP3D_MAX_STREAM_SIZE) {
    esp3d_log_e("Stream list is full");
    std::string text = esp3dTranslationService.translate(ESP3DLabel::error);
    text += ": S";
    text += std::to_string((uint8_t)ESP3DGcodeHostError::list_full);
    esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                    text.c_str());
    return false;
  }
  // Create a new stream
  ESP3DGcodeStream* new_stream =
      (ESP3DGcodeStream*)malloc(sizeof(ESP3DGcodeStream));
  if (new_stream == nullptr) {
    esp3d_log_e("Failed to allocate memory for new stream");
    return false;
  }
  esp3d_log("New stream type: %d", static_cast<uint8_t>(type));

  new_stream->type = type;
  new_stream->id = esp3d_hal::millis();
  new_stream->auth_type = auth_type;
  new_stream->cursorPos = 0;
  new_stream->processedSize = 0;
  new_stream->totalSize = 0;
  new_stream->active = false;
  new_stream->state = ESP3DGcodeStreamState::start;
  new_stream->dataStream = (char*)malloc(strlen(data) + 1);
  if (new_stream->dataStream == nullptr) {
    esp3d_log_e("Failed to allocate memory for new stream");
    free(new_stream);
    return false;
  }
  strcpy(new_stream->dataStream, data);
  esp3d_log("New stream data: %s", new_stream->dataStream);
  // Sanity check for multiple commands
  if (type == ESP3DGcodeHostStreamType::multiple_commands) {
    for (int i = 0; i < strlen(new_stream->dataStream); i++) {
      if (new_stream->dataStream[i] == ';') {
        new_stream->dataStream[i] = '\n';
      }
    }
  }

  if (!_pushBackGCodeStream(new_stream, !executeFirst)) {
    esp3d_log_e("Failed to add stream");
    free(new_stream->dataStream);
    free(new_stream);
    return false;
  }

  return true;
}

bool ESP3DGCodeHostService::_pushBackGCodeStream(ESP3DGcodeStream* stream,
                                                 bool is_stream) {
  bool res = false;
  if (is_stream) {
    esp3d_log("Add stream file");
    if (_streams_list_mutex) {
      if (pthread_mutex_lock(&_streams_list_mutex) == 0) {
        uint size = _streams.size();
        if (size < ESP3D_MAX_STREAM_SIZE) {
          _streams.push_back(stream);
          res = true;
        } else {
          esp3d_log_e("Stream list is full");
        }
        uint size_after = _streams.size();
        if (size_after == size + 1) {
          esp3d_log("Stream added nb=%d", size_after);
        } else {
          esp3d_log_e("Failed to add stream");
          res = false;
        }
        pthread_mutex_unlock(&_streams_list_mutex);
      } else {
        esp3d_log_e("Failed to lock stream list mutex");
      }
    } else {
      esp3d_log_e("Stream list mutex not initialized");
    }
  } else {
    esp3d_log("Add script/command");
    if (_scripts_list_mutex) {
      if (pthread_mutex_lock(&_scripts_list_mutex) == 0) {
        uint size = _scripts.size();
        if (size < ESP3D_MAX_STREAM_SIZE) {
          _scripts.push_back(stream);
          res = true;
        } else {
          esp3d_log_e("Script list is full");
        }
        uint size_after = _scripts.size();
        if (size_after == size + 1) {
          esp3d_log("Script added nb=%d", size_after);
        } else {
          esp3d_log_e("Failed to add script");
          res = false;
        }
        pthread_mutex_unlock(&_scripts_list_mutex);
      } else {
        esp3d_log_e("Failed to lock script list mutex");
      }
    } else {
      esp3d_log_e("Script list mutex not initialized");
    }
  }
  return res;
}

bool ESP3DGCodeHostService::_popFrontGCodeStream(bool is_stream) {
  bool res = false;
  if (is_stream) {
    esp3d_log("Pop stream file");
    if (_streams_list_mutex) {
      if (pthread_mutex_lock(&_streams_list_mutex) == 0) {
        if (_streams.size() != 0) {
          ESP3DGcodeStream* stream = _streams.front();
          _streams.pop_front();
          free(stream->dataStream);
          free(stream);
          res = true;
        } else {
          res = false;
        }
        pthread_mutex_unlock(&_streams_list_mutex);
      } else {
        esp3d_log_e("Failed to lock stream list mutex");
      }
    } else {
      esp3d_log_e("Stream list mutex not initialized");
    }
  } else {
    esp3d_log("Pop script/command");
    if (_scripts_list_mutex) {
      if (pthread_mutex_lock(&_scripts_list_mutex) == 0) {
        if (_scripts.size() != 0) {
          ESP3DGcodeStream* stream = _scripts.front();
          _scripts.pop_front();
          free(stream->dataStream);
          free(stream);
          res = true;
        } else {
          res = false;
        }
        pthread_mutex_unlock(&_scripts_list_mutex);
      } else {
        esp3d_log_e("Failed to lock script list mutex");
      }
    } else {
      esp3d_log_e("Script list mutex not initialized");
    }
  }
  return res;
}

bool ESP3DGCodeHostService::abort() {
  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, _xAbortNotifyIndex);
  return true;
}

bool ESP3DGCodeHostService::pause() {
  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, _xPauseNotifyIndex);
  return true;
}

bool ESP3DGCodeHostService::resume() {
  xTaskNotifyGive(_xHandle);
  xTaskNotifyGiveIndexed(_xHandle, _xResumeNotifyIndex);
  return true;
}

/// @brief Opens the file assosciated with the provided file stream, and seeks
/// to the correct position.
/// set the total size of the file if not already set.
/// @param stream Pointer to the file stream to be opened.
/// @return True if file opened successfully.
bool ESP3DGCodeHostService::_openFile(ESP3DGcodeStream* stream) {
  esp3d_log("File name is %s", stream->dataStream);
  _file_buffer_length = 0;
  if (globalFs.accessFS(stream->dataStream)) {
    if (globalFs.exists(stream->dataStream)) {
      esp3d_log("File exists");
      _file_handle = globalFs.open(stream->dataStream, "r");
      if (_file_handle != nullptr) {
        if (_current_stream_ptr->cursorPos != 0) {
          if (fseek(_file_handle, (long)stream->cursorPos,
                    SEEK_SET) !=
              0) {  // this would need adjusting if concurrrent file access is
                    // implemented (seek to buffer end instead)
            esp3d_log_e("Failed to seek to correct position in file: %s",
                        stream->dataStream);
            _error = ESP3DGcodeHostError::cursor_out_of_range;
            globalFs.releaseFS(stream->dataStream);
            return false;
          }
        }
        esp3d_log("File opened");
        // get file size if not set
        if (stream->totalSize == 0) {
          struct stat file_stat;
          if (globalFs.stat(stream->dataStream, &file_stat) == -1) {
            esp3d_log_e("Failed to get file size");
            _error = ESP3DGcodeHostError::file_system;
            globalFs.releaseFS(stream->dataStream);
            return false;
          }
          if (file_stat.st_size > 0) {
            stream->totalSize = file_stat.st_size;
          } else {
            esp3d_log_e("File size is 0");
            _error = ESP3DGcodeHostError::empty_file;
            globalFs.releaseFS(stream->dataStream);
            return false;
          }
        }
        _error = ESP3DGcodeHostError::no_error;
        return true;
      } else {
        esp3d_log_e("Failed to open file");
        _error = ESP3DGcodeHostError::access_denied;
        globalFs.releaseFS(stream->dataStream);
      }
    } else {
      esp3d_log_e("File does not exist");
      _error = ESP3DGcodeHostError::file_not_found;
      globalFs.releaseFS(stream->dataStream);
    }

  } else {
    esp3d_log_e("Can't access FS");
    _error = ESP3DGcodeHostError::file_system;
  }
  return false;
}

/// @brief Closes the File assosciated with the provided file stream.
/// @param stream Pointer to the file stream to be closed.
/// @return True if file closed and FS released successfully.
bool ESP3DGCodeHostService::_closeFile(ESP3DGcodeStream* stream) {
  if (_file_handle == nullptr) {
    esp3d_log_w("No file to close");
    return false;
  }
  esp3d_log("Closing File: %s", stream->dataStream);
  globalFs.close((_file_handle), stream->dataStream);
  _file_handle = nullptr;
  _file_buffer_length = 0;
  globalFs.releaseFS(stream->dataStream);
  return true;
}

// ##################### Stream Handling Functions ########################

bool ESP3DGCodeHostService::_startStream(ESP3DGcodeStream* stream) {
  _error = ESP3DGcodeHostError::no_error;
  esp3d_log("Starting stream");
  _file_buffer_length = 0;
  _current_command_str = "";
  if (isFileStream(stream)) {
    if (_file_handle) {
      esp3d_log("File handle is not null, close it");
      _closeFile(stream);
      esp3d_log_e("Previous File handle not closed!");
    }
    // only for main stream we need to reset the command line number
    // and set the file name and stream status
    if (stream->totalSize == 0 &&
        (stream->type == ESP3DGcodeHostStreamType::fs_stream ||
         stream->type == ESP3DGcodeHostStreamType::sd_stream)) {
      // cannot be active a because the command line reset is not done
      // and so we will need to send a line number reset command
      stream->active = false;
      std::string cmd = esp3dGcodeParser.getFwCommandString(
          FW_GCodeCommand::reset_stream_numbering);
      // add command on top of current stream
      esp3d_log("Add command: %s", cmd.c_str());
      _add_stream(cmd.c_str(), stream->auth_type, true);
      _command_number = 0;
      esp3dTftValues.set_string_value(ESP3DValuesIndex::job_status,
                                      "processing");
      esp3dTftValues.set_string_value(ESP3DValuesIndex::file_name,
                                      stream->dataStream);
    }
    // open file take care of cursor position
    // and total size
    if (!_openFile(stream)) {
      esp3d_log_e("Failed to open file");
      _error = ESP3DGcodeHostError::file_system;
      return false;
    }
    return true;
  } else if (isCommandStream(stream)) {
    stream->totalSize = strlen(stream->dataStream);
    esp3d_log("Is command stream");
    return true;
  }
  _error = ESP3DGcodeHostError::unknow;
  esp3d_log_e("Unknown stream type");
  return false;
}

/// @brief End the current stream, freeing any allocated memory and removing
/// scripts from the queue.
/// @param stream Pointer to the stream to end and free.
/// @return true if stream is ended and freed successfully.
bool ESP3DGCodeHostService::_endStream(ESP3DGcodeStream* stream) {
  if (!stream) {
    esp3d_log_e("Stream is null");
    return false;
  }
  esp3d_log("Freeing stream");
  ESP3DGcodeStream* stream_to_end = nullptr;
  bool is_stream = (stream->type == ESP3DGcodeHostStreamType::fs_stream ||
                    stream->type == ESP3DGcodeHostStreamType::sd_stream);
  if (is_stream) {
    stream_to_end = getCurrentMainStream();
  } else {
    stream_to_end = _get_front_script();
  }
  if (stream_to_end != stream) {
    esp3d_log_e("Stream to end is not the current stream");
    return false;
  }
  if (isFileStream(stream)) {
    esp3d_log("Close stream");
    _closeFile(stream);
  }
  if (!_popFrontGCodeStream(is_stream)) {
    esp3d_log_e("Failed to pop stream");
    return false;
  }
  return true;
}

/// @brief Read the next command line from the provided stream into
/// _currentCommand. Skips past comments and empty lines.
/// @param stream Pointer to the command or file stream to be read from.
/// @return True if command is read, False if no command read (end of
/// stream).
bool ESP3DGCodeHostService::_readNextCommand(ESP3DGcodeStream* stream) {
  // No need to put this in class scope
  static size_t buffer_cursor = 0;
  bool need_search_command = true;
  _error = ESP3DGcodeHostError::no_error;
  esp3d_log("Reading next command");
  if (stream->type == ESP3DGcodeHostStreamType::single_command) {
    esp3d_log("Single command");
    // this is manual command it should not have any comment
    _current_command_str = stream->dataStream;
    stream->cursorPos = strlen(stream->dataStream);
    need_search_command = false;
  } else if (stream->type == ESP3DGcodeHostStreamType::multiple_commands) {
    esp3d_log("Multiple command");
    // read from buffer = dataStream
    // reset content
    _current_command_str = "";
    for (int i = stream->cursorPos; i < strlen(stream->dataStream); i++) {
      stream->cursorPos++;
      if (stream->dataStream[i] == '\n') {
        need_search_command = false;
        esp3d_log("End of line");
        break;
      }
      _current_command_str += stream->dataStream[i];
    }
    esp3d_log("Command read: %s", _current_command_str.c_str());
  } else if (isFileStream(stream)) {
    _current_command_str = "";
    esp3d_log("File commands cursor pos is %lld and current command is %s",
              stream->cursorPos, _current_command_str.c_str());
    if (_file_handle == nullptr) {
      esp3d_log_e("No file handle");
      _error = ESP3DGcodeHostError::file_system;
      return false;
    }

    // read from file
    while (need_search_command) {
      esp3d_log("Buffer pos: %d, in buffer of %d,  for %lld/%lld",
                buffer_cursor, _file_buffer_length, stream->cursorPos,
                stream->totalSize);
      if (buffer_cursor >= _file_buffer_length) {
        // we need refill buffer from file
        _file_buffer_length = 0;
      }
      // read from file
      if (_file_buffer_length == 0) {
        esp3d_log("Buffer is empty, read from file");
        buffer_cursor = 0;
        _file_buffer_length = fread(_file_buffer, sizeof(char),
                                    STREAM_CHUNK_SIZE - 1, _file_handle);
        _file_buffer[_file_buffer_length] = 0;

        if (_file_buffer_length == 0) {
          esp3d_log_e("Failed to read from file");
          if (stream->cursorPos < stream->totalSize) {
            esp3d_log_e("No more data available, but file size is not reached");
            _error = ESP3DGcodeHostError::file_system;
            return false;
          }
          // we reached the end of the file, so no need to search for a command
          // any more
          //  we are done even no final '\n' was found
          need_search_command = false;
        } else {
          // Note displaying the buffer can be problematic as it can contain \n
          // and \r and so it will be displayed on multiple lines and log char
          // won't be displayed on each line, which may confuse the simulator
          // taking the log as command
          // so may need to format the buffer before displaying it
          //  esp3d_log("Got %d bytes from file: *%s*", _file_buffer_length,
          //              _file_buffer);
        }
      }
      // we did not get the end of line, so we need to search for a command in
      // the buffer
      if (need_search_command) {
        esp3d_log("Parsing buffer to add to command %s",
                  _current_command_str.c_str());
        for (buffer_cursor = buffer_cursor; buffer_cursor < _file_buffer_length;
             buffer_cursor++) {
          // What ever we read it increase the cursor pos
          (stream->cursorPos)++;
          // esp3d_log("Parsing buffer at index:%d, cursor is now %lld",
          //             buffer_cursor, stream->cursorPos);
          //   do not add the `\n` on purpose for triming the command
          if (_file_buffer[buffer_cursor] == '\n' ||
              _file_buffer[buffer_cursor] == '\r') {
            esp3d_log("End of line %s found", _current_command_str.c_str());
            // is it en empty line ?
            //  if yes we need to continue to read
            if (_current_command_str.length() == 0) {
              esp3d_log("Empty line, continue, cursor is now %lld/%lld",
                        stream->cursorPos, stream->totalSize);
              continue;
            } else {
              buffer_cursor++;
              esp3d_log("Command found, is now: *%s* of %d bytes",
                        _current_command_str.c_str(),
                        _current_command_str.length());
              need_search_command = false;
              break;
            }
          }
          // fill the command string
          _current_command_str += _file_buffer[buffer_cursor];
          // esp3d_log("Command is now: %s", _current_command_str.c_str());
          if (_current_command_str.length() > MAX_COMMAND_LENGTH) {
            esp3d_log_e("Command too long > 255, %s",
                        _current_command_str.c_str());
            // is that necessary ? as we are not going to send it
            buffer_cursor++;
            _error = ESP3DGcodeHostError::command_too_long;
            return false;
          }
        }
        // seems we did not find any command end line
        if (need_search_command) {
          esp3d_log("No end of line found %lld/%lld", stream->cursorPos,
                    stream->totalSize);
          if (stream->cursorPos == stream->totalSize) {
            esp3d_log("End of file");
            // no need to search anymore
            need_search_command = false;
          } else {
            esp3d_log("Continue to read from file for %s",
                      _current_command_str.c_str());
            // we continue to read from file
            _file_buffer_length = 0;
          }
        }
      }
    }
  } else {
    esp3d_log_e("Unknown stream type");
    _error = ESP3DGcodeHostError::unknow;
    return false;
  }

  if (_current_command_str.length() == 0) {
    esp3d_log("No command read");
    return false;
  }

  if (stream->type == ESP3DGcodeHostStreamType::multiple_commands &&
      stream->cursorPos >= stream->totalSize) {
    esp3d_log("End of multiple commands stream reached");
    need_search_command = false;
  }

  esp3d_log("Command is currently %s", _current_command_str.c_str());
  if (need_search_command) {
    esp3d_log("No end line neither end of file, need to continue to read");
    return false;
  }
  esp3d_log("Cursor pos is now %lld / %lld, %lld, buffer cursor is %d/%d",
            stream->cursorPos, stream->totalSize,
            100 * stream->cursorPos / stream->totalSize, buffer_cursor,
            _file_buffer_length);
  _current_command_str = esp3d_string::str_trim(_current_command_str.c_str());
  esp3d_log("Trimmed command read: %s", _current_command_str.c_str());
  if (_current_command_str.length() == 0) {
    esp3d_log("No command read %lld/%lld, on buffer %d/%d", stream->cursorPos,
              stream->totalSize, buffer_cursor, _file_buffer_length);

    return false;
  }
  esp3d_log("Command read: %s of %d bytes", _current_command_str.c_str(),
            _current_command_str.length());
  return true;
}

/// @brief Calculate a checksum for the command provided. (Use crc.h
/// instead?)
/// @param command Const Char Pointer to a buffer containing the null
/// terminated command.
/// @param commandSize The number of bytes of data in the command.
/// @return The checksum as an unsigned integer.
uint8_t ESP3DGCodeHostService::_Checksum(const char* command,
                                         uint32_t commandSize) {
  uint8_t checksum_val = 0;
  if (command == NULL) {
    return 0;
  }
  for (uint32_t i = 0; i < commandSize; i++) {
    checksum_val = checksum_val ^ ((uint8_t)command[i]);
  }
  return checksum_val;
}

// maybe rejig to use _currentCommand and cmdno (no vars necessary) //maybe
// absorb _Checksum() //use esp32 crc

/// @brief Apply a line number and checksum to the command referred to by
/// the pointer.
/// @param result_buffer Pointer to the result buffer
/// @param max_result_size Size of the result buffer
/// @param command Pointer to the command to be checksummed.
/// @param commandnb Line number of the command.
/// @return True if altered. False if command is empty or overflow result
/// buffer
bool ESP3DGCodeHostService::_CheckSumCommand(char* result_buffer,
                                             size_t max_result_size,
                                             const char* command,
                                             uint32_t commandnb) {
  if (strlen(command) == 0) {
    return false;
  }
  std::string cmd = "N" + std::to_string(commandnb) + " " + command;
  uint8_t chksm = _Checksum(cmd.c_str(), cmd.length());
  cmd += "*" + std::to_string(chksm) + "\n";
  if (cmd.length() > max_result_size) {
    esp3d_log_e("Command too long");
    return false;
  }
  strcpy(result_buffer, cmd.c_str());
  return true;
}

bool ESP3DGCodeHostService::_parseResponse(ESP3DMessage* rx) {
  esp3d_log("Parse response %s", (char*)(rx->data));
  ESP3DDataType response_type = esp3dGcodeParser.getType((char*)(rx->data));
  esp3d_log("Response type %d", static_cast<uint8_t>(response_type));
  switch (response_type) {
    case ESP3DDataType::ack:  // ack
      _startTimeout = esp3d_hal::millis();
      esp3d_log("Reset timeout");
      esp3d_log("Got ack %s", esp3d_string::str_trim((char*)rx->data));
      esp3d_log("for %s", esp3d_string::str_trim(_current_command_str.c_str()));
      if (_awaitingAck) {
        esp3d_log("When having awaiting ack");
        // we got an ack for the current command
        _awaitingAck = false;
        // save one cycle for single command
        if (_current_stream_ptr->type ==
            ESP3DGcodeHostStreamType::single_command) {
          _setStreamState(ESP3DGcodeStreamState::end);
        } else {
          _setStreamState(ESP3DGcodeStreamState::ready_to_read_cursor);
          _current_command_str = "";
        }

      } else {
        esp3d_log("Got ack but out of the query");
      }
      break;
    case ESP3DDataType::status:  // can be busy or idle
      // just reset the timeout
      _startTimeout = esp3d_hal::millis();
      esp3d_log("Reset timeout");
      break;
    case ESP3DDataType::error:  // error
      esp3d_log_e("Got Error: %s", ((char*)(rx->data)));
      if (_awaitingAck) {
        _awaitingAck = false;
        // TODO: handle error ?
        // e.g: Marlin raise error first then ask for resend
      } else {
        std::string text = esp3dTranslationService.translate(ESP3DLabel::error);
        text += ": P";
        text += esp3dGcodeParser.getLastError();
        esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                        text.c_str());
        esp3d_log("Got error but out of the query");
      }
      break;
    case ESP3DDataType::resend:  // resend
      // use _current_command_str and resend it to the printer
      esp3d_log("Got resend");
      _startTimeout = esp3d_hal::millis();
      esp3d_log("Reset timeout");
      if (_awaitingAck) {
        _resend_command_number = esp3dGcodeParser.getLineResend();
        _resend_command_counter++;
        esp3d_log("Got resend %lld", _resend_command_number);
        _setStreamState(ESP3DGcodeStreamState::resend_gcode_command);
      } else {
        esp3d_log("Got resend but out of the query");
      }
      break;
    default:  // esp_command, gcode, response,  comment, empty_line,
      // emergency_command, unknown break
      // the response is ignored by gcode host
      esp3d_log("Got useless response: %s", (char*)(rx->data));
      return false;
      break;
  }
  return true;
}

bool ESP3DGCodeHostService::_processRx(ESP3DMessage* rx) {
  if (_outputClient == ESP3DClientType::no_client) {
    esp3d_log("Output client not set, can't send: %s", (char*)(rx->data));
  } else if (rx->origin == _outputClient) {
    esp3d_log("Stream got data: %s from %d", (char*)(rx->data),
              static_cast<uint8_t>(rx->origin));
    if (_connection_lost) {
      std::string text = esp3dTranslationService.translate(
          ESP3DLabel::communication_recovered);
      esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                      text.c_str());
    }
    _connection_lost = false;

    _parseResponse(rx);
  } else {
    esp3d_log("Sending message from: %d to %d",
              static_cast<uint8_t>(rx->origin),
              static_cast<uint8_t>(rx->target));
    addStream((const char*)rx->data, rx->size, rx->authentication_level);
  }
  deleteMsg(rx);
  return true;
}

// The getState function is used to get the current state of the host
// streaming main stream, not commands
//  0 - idle
//  1 - processing
//  2 - paused
//  3 - error
ESP3DGcodeHostState ESP3DGCodeHostService::getState() {
  // Check if there is a main stream in the queue
  ESP3DGcodeStream* stream = getCurrentMainStream();
  ESP3DGcodeHostState state = ESP3DGcodeHostState::idle;
  // if main stream check if it is paused or processing
  if (stream) {
    if (stream->state == ESP3DGcodeStreamState::paused) {
      state = ESP3DGcodeHostState::paused;
    } else {
      state = ESP3DGcodeHostState::processing;
    }
  }
  // there are no main stream in the queue
  return state;
}

// Get top active stream even it is paused
ESP3DGcodeStream* ESP3DGCodeHostService::getCurrentMainStream() {
  if (_streams.empty()) return nullptr;
  return _streams.front();
}

ESP3DGcodeHostError ESP3DGCodeHostService::getErrorNum() { return _error; }

ESP3DGCodeHostService::ESP3DGCodeHostService() {
  _started = false;
  _xHandle = NULL;
  _current_stream_ptr = NULL;
  _current_main_stream_ptr = NULL;
}
ESP3DGCodeHostService::~ESP3DGCodeHostService() { end(); }

void ESP3DGCodeHostService::process(ESP3DMessage* msg) {
  esp3d_log("Add message to queue");
  //  the flush is used to avoid the queue to be full
  if (!addRxData(msg)) {
    flush();
    if (!addRxData(msg)) {
      esp3d_log_e("Cannot add msg to client queue");
      deleteMsg(msg);
    }
  } else {
    // flush();
  }
  //}
}

bool ESP3DGCodeHostService::begin() {
  end();
  if (pthread_mutex_init(&_rx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for rx failed");
    return false;
  }
  setRxMutex(&_rx_mutex);

  if (pthread_mutex_init(&_tx_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for tx failed");
    return false;
  }
  setTxMutex(&_tx_mutex);

  if (pthread_mutex_init(&_streams_list_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for streams list failed");
    return false;
  }

  if (pthread_mutex_init(&_scripts_list_mutex, NULL) != 0) {
    esp3d_log_e("Mutex creation for scripts list failed");
    return false;
  }

  // Task is never stopped so no need to kill the task from outside

  // this is done once because it is not possible to change the output client
  // after the fw is started
  _outputClient = esp3dCommands.getOutputClient(true);
  esp3d_log("Output client is: %d", static_cast<uint8_t>(_outputClient));
  BaseType_t res = xTaskCreatePinnedToCore(
      esp3d_gcode_host_task, "esp3d_gcode_host_task",
      ESP3D_GCODE_HOST_TASK_SIZE, NULL, ESP3D_GCODE_HOST_TASK_PRIORITY,
      &_xHandle, ESP3D_GCODE_HOST_TASK_CORE);

  if (res == pdPASS && _xHandle) {
    esp3d_log("Created GCode Host Task");
    esp3d_hal::wait(100);
    esp3d_log("GCode Host service started");
    _started = true;
    flush();
    return true;
  } else {
    esp3d_log_e("GCode Host Task creation failed");
    _started = false;
    return false;
  }
}

bool ESP3DGCodeHostService::_stripCommand() {
  std::string cmd = "";
  for (uint i = 0; i < _current_command_str.length(); i++) {
    if (_current_command_str[i] == ';') {
      break;
    }
    cmd += _current_command_str[i];
  }
  _current_command_str = esp3d_string::str_trim(cmd.c_str());
  if (_current_command_str.length() == 0) {
    return false;
  }

  return true;
}

// to avoid undefined reference if list is empty
ESP3DGcodeStream* ESP3DGCodeHostService::_get_front_script() {
  if (_scripts.empty()) return nullptr;
  return _scripts.front();
}

//_current_stream_ptr: is current processed stream, it should be one of the
// below:
//
// 1 - _current_main_stream_ptr: is the main stream (first main stream in the
// queue) this one can be put in hold if the front stream is not the main
// stream it also can be paused
//
// 2 -_current_front_stream_ptr: is the front stream (first stream in the
// queue) this one must become the _current_stream_ptr when the current active
// stream is processed
void ESP3DGCodeHostService::_handle_stream_selection() {
  ESP3DGcodeStream* _current_front_stream_ptr = nullptr;
  // if list is empty nothing to do
  if (_scripts.empty() && _streams.empty()) {
    // esp3d_log("No stream to handle");
    //  reset all pointers as sanity check
    _current_main_stream_ptr = nullptr;
    _current_stream_ptr = nullptr;
    _current_front_stream_ptr = nullptr;
    return;
  }

  // update the front stream
  _current_front_stream_ptr = _get_front_script();

  // if we are not streaming the front stream
  if (_current_stream_ptr == nullptr) {
    esp3d_log("No current stream, get the front one");
    _current_stream_ptr = _current_front_stream_ptr;
  }
  // check what is the main stream
  _current_main_stream_ptr = getCurrentMainStream();

  // if no script in the queue, the main stream is the current stream
  if (_current_stream_ptr == nullptr) {
    esp3d_log("No current stream, get the main stream as default");
    _current_stream_ptr = _current_main_stream_ptr;
  }

  // this should not happen, but just in case
  if (_current_stream_ptr->type == ESP3DGcodeHostStreamType::unknown ||
      _current_stream_ptr->type == ESP3DGcodeHostStreamType::invalid) {
    esp3d_log("Current stream is invalid, skip it");
    _setStreamState(ESP3DGcodeStreamState::error);
    _error = ESP3DGcodeHostError::unknow;
    _current_stream_ptr->active = true;
    return;
  }

  // only file stream  can be interrupted, scripts / command streams cannot
  if (!(_current_stream_ptr->type == ESP3DGcodeHostStreamType::fs_stream) &&
      !(_current_stream_ptr->type == ESP3DGcodeHostStreamType::sd_stream)) {
    /* esp3d_log("Current stream is not a file stream  %lld, state:%s : type:
       %s", _current_stream_ptr->id,
               ESP3DGcodeStreamStateStr[static_cast<uint8_t>(
                   _current_stream_ptr->state)],
               ESP3DGcodeHostStreamTypeStr[static_cast<uint8_t>(
                   _current_stream_ptr->type)]);*/
    /*esp3d_log("keep it always active, command:%s",
              _current_command_str.c_str());*/
    _current_stream_ptr->active = true;
    return;
  }

  // current stream cannot be interrupted unless it is paused or ready to read
  // next command
  if (!(_current_stream_ptr->state ==
        ESP3DGcodeStreamState::ready_to_read_cursor) &&
      !(_current_stream_ptr->state == ESP3DGcodeStreamState::paused)) {
    esp3d_log(
        "Current stream is processing %lld, state:%s : type: %s , command: "
        "%s, "
        "keep it active",
        _current_stream_ptr->id,
        ESP3DGcodeStreamStateStr[static_cast<uint8_t>(
            _current_stream_ptr->state)],
        ESP3DGcodeHostStreamTypeStr[static_cast<uint8_t>(
            _current_stream_ptr->type)],
        _current_command_str.c_str());
    _current_stream_ptr->active = true;
    return;
  }

  // the current stream is main one but there are other streams in the queue
  // in front of it
  if (_current_stream_ptr == _current_main_stream_ptr &&
      _current_stream_ptr != _current_front_stream_ptr &&
      _current_front_stream_ptr) {
    if (_current_main_stream_ptr) {
      esp3d_log(
          "Put main stream in hold because "
          "there are other streams in front of it");
      _current_main_stream_ptr->active = false;
      // close the file if needed
      if (_file_handle) {
        esp3d_log("File handle is not null, close it");
        _closeFile(_current_main_stream_ptr);
      }
    }
  }
  if (_current_front_stream_ptr) {
    _current_stream_ptr = _current_front_stream_ptr;
    esp3d_log("Current stream is now the front stream");
  }

  if (_current_stream_ptr == _current_main_stream_ptr &&
      _file_handle == nullptr &&
      _current_main_stream_ptr->state != ESP3DGcodeStreamState::paused) {
    esp3d_log("Current stream is the main stream, open the file");
    _setMainStreamState(ESP3DGcodeStreamState::start);
  }

  _current_stream_ptr->active = true;

  esp3d_log("Current stream is now %lld, state:%s : type: %s , command: %s",
            _current_stream_ptr->id,
            ESP3DGcodeStreamStateStr[static_cast<uint8_t>(
                _current_stream_ptr->state)],
            ESP3DGcodeHostStreamTypeStr[static_cast<uint8_t>(
                _current_stream_ptr->type)],
            _current_command_str.c_str());
}

// all changes of state must use this one so sanity checks can be done
// if necessary
// returns true if state change is successful
bool ESP3DGCodeHostService::_setStreamState(ESP3DGcodeStreamState state) {
  bool res = false;
  if (_streams_list_mutex) {
    if (pthread_mutex_lock(&_streams_list_mutex) == 0) {
      if (_current_stream_ptr) {
        _current_stream_ptr->state = state;
        res = true;
      }
      pthread_mutex_unlock(&_streams_list_mutex);
    } else {
      esp3d_log("Failed to lock stream list mutex");
    }
  } else {
    esp3d_log("Stream list mutex not initialized");
  }
  return res;
}

bool ESP3DGCodeHostService::_setMainStreamState(ESP3DGcodeStreamState state) {
  bool res = false;
  if (_streams_list_mutex) {
    if (pthread_mutex_lock(&_streams_list_mutex) == 0) {
      if (_current_main_stream_ptr) {
        _current_main_stream_ptr->state = state;
        res = true;
      }
      pthread_mutex_unlock(&_streams_list_mutex);
    } else {
      esp3d_log("Failed to lock stream list mutex");
    }
  } else {
    esp3d_log("Stream list mutex not initialized");
  }
  return res;
}

// this state is not applied immediately, but is used to determine the next
// state when reading the next command
bool ESP3DGCodeHostService::_setStreamRequestState(
    ESP3DGcodeStreamState state) {
  if (_current_main_stream_ptr) {
    _requested_state = state;
    return true;
  }
  _requested_state = ESP3DGcodeStreamState::undefined;
  return false;
}

// Give the current state of the active stream, if any
ESP3DGcodeStreamState ESP3DGCodeHostService::_getStreamState() {
  ESP3DGcodeStreamState state = ESP3DGcodeStreamState::undefined;
  if (_streams_list_mutex) {
    if (pthread_mutex_lock(&_streams_list_mutex) == 0) {
      if (_current_stream_ptr) {
        state = _current_stream_ptr->state;
      }
      pthread_mutex_unlock(&_streams_list_mutex);
    } else {
      esp3d_log("Failed to lock stream list mutex");
    }
  } else {
    esp3d_log("Stream list mutex not initialized");
  }
  return state;
}

// Handle the notifications
// be sure sdkconfig has CONFIG_FREERTOS_TASK_NOTIFICATION_ARRAY_ENTRIES>=3
void ESP3DGCodeHostService::_handle_notifications() {
  // esp3d_log("Handle notifications");
  // entry 0
  if (ulTaskNotifyTake(pdTRUE, 0)) {
    ESP3DGcodeStreamState state = ESP3DGcodeStreamState::undefined;
    ESP3DGcodeStream* stream = getCurrentMainStream();
    if (stream) {
      state = stream->state;
    }
    if (ulTaskNotifyTakeIndexed(_xPauseNotifyIndex, pdTRUE, 0)) {
      esp3d_log("Received pause notification");
      if (!_setStreamRequestState(ESP3DGcodeStreamState::pause)) {
        esp3d_log_e("Failed to request pause stream");
      } else {  // do not wait to set the state to paused to avoid to
                // confuse the user
        esp3dTftValues.set_string_value(ESP3DValuesIndex::job_status, "paused");
      }
    }

    if (ulTaskNotifyTakeIndexed(_xResumeNotifyIndex, pdTRUE, 0)) {
      esp3d_log("Received resume notification");
      if (state == ESP3DGcodeStreamState::paused) {
        if (!_setMainStreamState(ESP3DGcodeStreamState::resume)) {
          esp3d_log_e("Failed to resume stream");
        } else {  // do not wait to set the state to processing to avoid to
                  // confuse the user
          esp3dTftValues.set_string_value(ESP3DValuesIndex::job_status,
                                          "processing");
        }
      } else {
        esp3d_log("No paused stream - nothing to resume");
      }
    }
    if (ulTaskNotifyTakeIndexed(_xAbortNotifyIndex, pdTRUE, 0)) {
      esp3d_log_d("Received abort notification");
      if (state != ESP3DGcodeStreamState::undefined) {
        if (!_setStreamRequestState(ESP3DGcodeStreamState::abort)) {
          esp3d_log_e("Failed to abort stream");
        } else {  // do not wait to set the state to processing to avoid to
                  // confuse the user
          esp3dTftValues.set_string_value(ESP3DValuesIndex::job_status, "idle");
        }
      }
    }
  }
}

// Handle the messages in the queue
void ESP3DGCodeHostService::_handle_msgs() {
  // esp3d_log("Handle messages");
  while (getRxMsgsCount() > 0) {
    ESP3DMessage* msg = popRx();
    esp3d_log("RX popped");
    if (msg) {
      _processRx(msg);
    }
  }
}

// handle stream state changes
void ESP3DGCodeHostService::_handle_stream_states() {
  static std::string progress_str;
  static uint64_t last_progression_time = 0;
  static uint64_t last_ellapsedtime = 0;
  double new_progress;
  std::string new_progress_str;
  ESP3DRequest requestId = {.id = 0};

  if (_current_stream_ptr == nullptr) {
    return;  // nothing to be done if no stream it will changed in next
             // selection
  }
  ESP3DMessage* msg = nullptr;
  char buffer_str[MAX_COMMAND_LENGTH + 1] = {0};
  std::string text;

  if (!_current_stream_ptr->active) {
    esp3d_log("Stream not active");
    return;  // nothing to be done if stream is not active
  }
  // stream is active
  switch (_current_stream_ptr->state) {
      /////////////////////////////////////////////////////////
      // start
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::start:
      esp3d_log("Stream is starting");
      if (!_startStream(_current_stream_ptr)) {
        esp3d_log_e("Failed to start stream");
        _error = ESP3DGcodeHostError::file_system;
        _setStreamState(ESP3DGcodeStreamState::error);
      } else {
        if (!_current_stream_ptr->active) {
          esp3d_log("Stream not yet active, need reset line");
        }
        _setStreamState(ESP3DGcodeStreamState::ready_to_read_cursor);
      }
      break;

      /////////////////////////////////////////////////////////
      // ready to read cursor
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::ready_to_read_cursor:
      // ready to read next command
      esp3d_log("Stream is ready to read cursor");
      // check if we have a requested command to process before reading the
      // next
      if (_requested_state != (ESP3DGcodeStreamState::undefined)) {
        if (_requested_state == ESP3DGcodeStreamState::pause || _requested_state == ESP3DGcodeStreamState::abort) {
          // pause the stream
          _setStreamState(_requested_state);
        } else {
          // no valid requested state
          _setStreamState(ESP3DGcodeStreamState::read_cursor);
        }
        _requested_state = ESP3DGcodeStreamState::undefined;
      } else {
        // no requested command, read next command
        _setStreamState(ESP3DGcodeStreamState::read_cursor);
      }
      break;

      /////////////////////////////////////////////////////////
      // read cursor
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::read_cursor:
      // ready to read next command

      esp3d_log("Processed %lld bytes / %lld", _current_stream_ptr->cursorPos,
                _current_stream_ptr->totalSize);
      if (_current_stream_ptr->cursorPos !=
          _current_stream_ptr->processedSize) {
        _current_stream_ptr->processedSize = _current_stream_ptr->cursorPos;
        if (_current_stream_ptr->totalSize != 0 &&
            ((_current_stream_ptr->type ==
              ESP3DGcodeHostStreamType::fs_stream) ||
             (_current_stream_ptr->type ==
              ESP3DGcodeHostStreamType::sd_stream))) {
          if (esp3d_hal::millis() - last_ellapsedtime >
              ESP3D_REFRESH_INTERVAL) {
            last_ellapsedtime = esp3d_hal::millis();
            esp3dTftValues.set_string_value(
                ESP3DValuesIndex::job_duration,
                std::to_string(esp3d_hal::millis() - _current_stream_ptr->id)
                    .c_str());
          }

          new_progress = (1.0 * _current_stream_ptr->processedSize) /
                         _current_stream_ptr->totalSize;
          new_progress = (new_progress)*100;
          new_progress_str = esp3d_string::set_precision(
              std::to_string(new_progress).c_str(), 2);
          if (progress_str != new_progress_str &&
              new_progress_str != "100.00") {
            progress_str = new_progress_str;
            if (esp3d_hal::millis() - last_progression_time >
                ESP3D_REFRESH_INTERVAL) {
              last_progression_time = esp3d_hal::millis();
              esp3dTftValues.set_string_value(ESP3DValuesIndex::job_progress,
                                              progress_str.c_str());
            }
          }
        }
      }
      if (_current_stream_ptr->processedSize ==
          _current_stream_ptr->totalSize) {
        esp3d_log("Stream is finished");
        _setStreamState(ESP3DGcodeStreamState::end);
        break;
      }

      if (_readNextCommand(_current_stream_ptr)) {
        esp3d_log("Read next command: *%s*", _current_command_str.c_str());
        if (esp3dCommands.is_esp_command((uint8_t*)_current_command_str.c_str(),
                                         _current_command_str.length())) {
          esp3d_log("Command is an esp command");
          _setStreamState(ESP3DGcodeStreamState::send_esp_command);
          break;
        } else {
          // now string can be stripped if necessary
          esp3d_log("Stripping %d bytes for GCODE %s",
                    _current_command_str.length(),
                    _current_command_str.c_str());
          if (!_stripCommand()) {
            esp3d_log("Command is empty");
            // save one cycle for single command
            if (_current_stream_ptr->type ==
                ESP3DGcodeHostStreamType::single_command) {
              _setStreamState(ESP3DGcodeStreamState::end);
            } else {
              _setStreamState(ESP3DGcodeStreamState::ready_to_read_cursor);
            }
            break;
          } else {
            esp3d_log("Stripped command is now: %s, of %d bytes",
                      _current_command_str.c_str(),
                      _current_command_str.length());
          }
          _setStreamState(ESP3DGcodeStreamState::send_gcode_command);
        }
      } else {
        esp3d_log("Failed to read next command");
        // it is an error
        if (_error != ESP3DGcodeHostError::no_error) {
          _setStreamState(ESP3DGcodeStreamState::error);
        } else {
          // we reach end of file so let's set the state to end
          if (_current_stream_ptr->cursorPos >=
              _current_stream_ptr->totalSize) {
            _setStreamState(ESP3DGcodeStreamState::end);
          } else {  // keep reading
            esp3d_log("Keep reading next command");
            _setStreamState(ESP3DGcodeStreamState::ready_to_read_cursor);
          }
        }
      }

      break;
      /////////////////////////////////////////////////////////
      // resend_gcode_command
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::resend_gcode_command:
      if (_resend_command_counter >= ESP3D_MAX_RETRY) {
        esp3d_log_e("Too many resend, aborting");
        _error = ESP3DGcodeHostError::too_many_resend;
        _setStreamState(ESP3DGcodeStreamState::error);
        break;
      }
      if (_command_number == _resend_command_number) {
        _command_number = _resend_command_number - 1;
        _setStreamState(ESP3DGcodeStreamState::ready_to_read_cursor);
      } else {
        // Commmand is not the last one
        // how to handle this case ?
        _error = ESP3DGcodeHostError::number_mismatch;
        _setStreamState(ESP3DGcodeStreamState::error);
        break;
      }
      /* fall through */
    /////////////////////////////////////////////////////////
    // send_gcode_command
    /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::send_gcode_command:
      _startTimeout = esp3d_hal::millis();
      esp3d_log("Reset timeout");
      msg = nullptr;
      // do we need to forward to screen ?
      if (esp3dGcodeParser.forwardToScreen(_current_command_str.c_str())) {
        esp3d_log("Forwarding to screen %s", _current_command_str.c_str());
      }
      if (esp3dGcodeParser.isAckNeeded() &&
          (_current_stream_ptr->type == ESP3DGcodeHostStreamType::sd_stream ||
           _current_stream_ptr->type == ESP3DGcodeHostStreamType::fs_stream)) {
        _command_number++;
        esp3d_log("Command number: %lld, need checksum", _command_number);
        if (!_CheckSumCommand(buffer_str, MAX_COMMAND_LENGTH,
                              _current_command_str.c_str(), _command_number)) {
          esp3d_log_e("Failed to format command");
          _error = ESP3DGcodeHostError::memory_allocation;
          _setStreamState(ESP3DGcodeStreamState::error);
          break;
        }
        // the checksumed command integrate the final `\n` so no need to add
        // it or check if need to add `\n`
        esp3d_log("Sending : %d for %s", strlen(buffer_str), buffer_str);
        msg =
            newMsg(ESP3DClientType::stream, _outputClient, (uint8_t*)buffer_str,
                   strlen(buffer_str), _current_stream_ptr->auth_type);
      } else {
        // FIXME:
        // this part is arguable for command that are not command
        // and so do not have a \n at the end e.g: realtime commands from
        // grbl so it may be better to check if the command is a gcode
        // command or real time command and add the \n only if it is a gcode
        // command
        esp3d_log("Sending : %d for %s", _current_command_str.length(),
                  _current_command_str.c_str());
        if (_current_command_str[_current_command_str.length() - 1] != '\n') {
          _current_command_str += "\n";
        }
        msg = newMsg(ESP3DClientType::stream, _outputClient,
                     (uint8_t*)(_current_command_str.c_str()),
                     _current_command_str.length(),
                     _current_stream_ptr->auth_type);
      }
      if (msg) {
        esp3dCommands.process(msg);
        _awaitingAck = esp3dGcodeParser.hasAck(_current_command_str.c_str());
        esp3d_log("Awaiting ack: %s for %s", _awaitingAck ? "true" : "false",
                  _current_command_str.c_str());
        if (_awaitingAck) {
          esp3d_log("change state to Waiting for ack");
          _setStreamState(ESP3DGcodeStreamState::wait_for_ack);
          _startTimeout = esp3d_hal::millis();
        } else {
          esp3d_log("change state to read cursor");
          _current_command_str = "";
          if (_current_stream_ptr->type ==
              ESP3DGcodeHostStreamType::single_command) {
            _setStreamState(ESP3DGcodeStreamState::end);
          } else {
            _setStreamState(ESP3DGcodeStreamState::ready_to_read_cursor);
          }
        }
      } else {
        esp3d_log_e("Failed to create message");
        _error = ESP3DGcodeHostError::memory_allocation;
        _setStreamState(ESP3DGcodeStreamState::error);
      }
      break;

      /////////////////////////////////////////////////////////
      // send_esp_command
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::send_esp_command:
      if (_current_command_str[_current_command_str.length() - 1] != '\n') {
        _current_command_str += "\n";
      }
      msg =
          newMsg(ESP3DClientType::stream, ESP3DClientType::command,
                 (uint8_t*)(_current_command_str.c_str()),
                 _current_command_str.length(), _current_stream_ptr->auth_type);
      if (msg) {
        esp3dCommands.process(msg);
        // no ack to wait for esp commands
        // save one cycle for single command
        if (_current_stream_ptr->type ==
            ESP3DGcodeHostStreamType::single_command) {
          _setStreamState(ESP3DGcodeStreamState::end);
        } else {
          // continue and read the next command
          _setStreamState(ESP3DGcodeStreamState::ready_to_read_cursor);
        }
      } else {
        esp3d_log_e("Failed to create message");
        _error = ESP3DGcodeHostError::memory_allocation;
        _setStreamState(ESP3DGcodeStreamState::error);
      }
      break;
      /////////////////////////////////////////////////////////
      // wait for ack
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::wait_for_ack:
      // the ack is actually handled in the parse_response function
      // that will change state to read_cursor when ack is received
      // so here we only handle time out if any
      if (_startTimeout != 0 &&
          esp3d_hal::millis() - _startTimeout > ESP3D_COMMAND_TIMEOUT) {
        esp3d_log_e("Timeout waiting for ack");
        _error = ESP3DGcodeHostError::time_out;
        _setStreamState(ESP3DGcodeStreamState::error);
        if (!_connection_lost) {
          std::string text =
              esp3dTranslationService.translate(ESP3DLabel::communication_lost);
          esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                          text.c_str());
        }
        _connection_lost = true;
      }
      break;

      /////////////////////////////////////////////////////////
      // pause
      /////////////////////////////////////////////////////////
      // only valid for main stream
    case ESP3DGcodeStreamState::pause:
      if (!_setMainStreamState(ESP3DGcodeStreamState::paused)) {
        esp3d_log_e("Pause only valid for main stream");
        break;
      }
      _current_main_stream_ptr->active = false;
      if (_pause_script.length() > 0) {
        _add_stream(_pause_script.c_str(), _current_main_stream_ptr->auth_type,
                    true);
      }
      break;

      /////////////////////////////////////////////////////////
      // resume
      /////////////////////////////////////////////////////////
      // only valid for main stream
    case ESP3DGcodeStreamState::resume:
      if (!_setMainStreamState(ESP3DGcodeStreamState::start)) {
        esp3d_log_e("Resume only valid for main stream");
        break;
      }
      if (_resume_script.length() > 0) {
        _add_stream(_resume_script.c_str(), _current_main_stream_ptr->auth_type,
                    true);
      }
      break;

      /////////////////////////////////////////////////////////
      // abort
      /////////////////////////////////////////////////////////
      // only valid for main stream
    case ESP3DGcodeStreamState::abort:
      esp3d_log_d("Received abort notification");
      if (!_setMainStreamState(ESP3DGcodeStreamState::end)) {
        esp3d_log_e("Abort only valid for main stream");
        break;
      }
      esp3d_log_d("Aborting current stream");
      
      if (_stop_script.length() > 0) {
        esp3d_log_d("Adding stop script");
        _add_stream(_stop_script.c_str(), _current_main_stream_ptr->auth_type,
                    true);
      }
      break;

      /////////////////////////////////////////////////////////
      // end
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::end:
      esp3d_log("Stream is ended for type: %d, %s",
                static_cast<uint8_t>(_current_stream_ptr->type),
                _current_stream_ptr->dataStream);
      // sanity check, reset main stream pointer if it is the current stream
      if (_current_stream_ptr == _current_main_stream_ptr) {
        _current_main_stream_ptr = nullptr;
        esp3dTftValues.set_string_value(ESP3DValuesIndex::job_status, "idle");
        if (_current_stream_ptr->cursorPos >= _current_stream_ptr->totalSize) {
          esp3d_log("Stream is finished send 100");
          esp3dTftValues.set_string_value(ESP3DValuesIndex::job_progress,
                                          "100");
          esp3dTftValues.set_string_value(
              ESP3DValuesIndex::job_duration,
              std::to_string(esp3d_hal::millis() - _current_stream_ptr->id)
                  .c_str());
        }
        esp3dTftValues.set_string_value(
            ESP3DValuesIndex::job_duration,
            std::to_string(esp3d_hal::millis() - _current_stream_ptr->id)
                .c_str());
      }
      if (!_endStream(_current_stream_ptr)) {
        esp3d_log_e("Failed to end stream");
      }
      // current stream to null ptr
      _current_stream_ptr = nullptr;
      _current_command_str = "";
      _file_handle = nullptr;
      _file_buffer_length = 0;
      break;

      /////////////////////////////////////////////////////////
      // paused
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::paused:
      // Nothing to do because the stream is paused
      break;

      /////////////////////////////////////////////////////////
      // error
      /////////////////////////////////////////////////////////
    case ESP3DGcodeStreamState::error:
      // to be defined
      // should we abort the stream? or go to next command in stream if any
      // ?
      // how to notify to user ?
      esp3d_log_e("Stream is in error, cancel it %s , %s",
                  _current_command_str.c_str(),
                  _current_stream_ptr->dataStream);

      text = esp3dTranslationService.translate(ESP3DLabel::error);
      text += ": ";
      if (_current_stream_ptr->type ==
              ESP3DGcodeHostStreamType::single_command ||
          _current_stream_ptr->type ==
              ESP3DGcodeHostStreamType::multiple_commands) {
        text += esp3dTranslationService.translate(ESP3DLabel::command_error,
                                                  _current_command_str.c_str());
      } else if (_current_stream_ptr->type ==
                     ESP3DGcodeHostStreamType::fs_script ||
                 _current_stream_ptr->type ==
                     ESP3DGcodeHostStreamType::sd_script ||
                 _current_stream_ptr->type ==
                     ESP3DGcodeHostStreamType::fs_stream ||
                 _current_stream_ptr->type ==
                     ESP3DGcodeHostStreamType::sd_stream) {
        text += esp3dTranslationService.translate(
            ESP3DLabel::stream_error, _current_stream_ptr->dataStream);
      } else {
        text += esp3dTranslationService.translate(ESP3DLabel::streaming_error);
      }
      text += " S" + std::to_string(static_cast<uint8_t>(_error));
      text = esp3d_string::str_replace(text.c_str(), "\n", "");
      esp3dTftValues.set_string_value(ESP3DValuesIndex::status_bar_label,
                                      text.c_str());

      // send error message
      if (!esp3dCommands.dispatch(text.c_str(), ESP3DClientType::all_clients,
                                  requestId, ESP3DMessageType::unique,
                                  ESP3DClientType::stream,
                                  ESP3DAuthenticationLevel::admin)) {
        esp3d_log_e("Failed to send error message");
      }

#if ESP3D_NOTIFICATIONS_FEATURE
      text = "Error: ";
      if (_current_stream_ptr->type ==
              ESP3DGcodeHostStreamType::single_command ||
          _current_stream_ptr->type ==
              ESP3DGcodeHostStreamType::multiple_commands) {
        text += esp3dTranslationService.translate(ESP3DLabel::command_error,
                                                  _current_command_str.c_str());
      } else if (_current_stream_ptr->type ==
                     ESP3DGcodeHostStreamType::fs_script ||
                 _current_stream_ptr->type ==
                     ESP3DGcodeHostStreamType::sd_script ||
                 _current_stream_ptr->type ==
                     ESP3DGcodeHostStreamType::fs_stream ||
                 _current_stream_ptr->type ==
                     ESP3DGcodeHostStreamType::sd_stream) {
        text += esp3dTranslationService.translate(
            ESP3DLabel::stream_error, _current_stream_ptr->dataStream);
      } else {
        text += esp3dTranslationService.translate(ESP3DLabel::streaming_error);
      }
      text = esp3d_string::str_replace(text.c_str(), "\n", "");
      if (!esp3dNotificationsService.sendMSG(ESP3D_NOTIFICATION_TITLE,
                                             text.c_str())) {
        esp3d_log_e("Failed to send notification");
      }
#endif  // ESP3D_NOTIFICATIONS_FEATURE

      // abort the stream because we are in error and we do not know what to
      // do
      _setStreamState(ESP3DGcodeStreamState::end);
      break;
    default:
      break;
  };
}

void ESP3DGCodeHostService::handle() {
  if (!_started) {
    return;
  }
  // Check for notifications, set the current stream state
  _handle_notifications();

  // Handle the stream
  _handle_stream_selection();

  // Handle the state machine
  _handle_stream_states();

  // esp3d_log("Host state: %d", static_cast<uint8_t>(state));
  // handle the messages in the queue
  _handle_msgs();
}

void ESP3DGCodeHostService::flush() {  // should only be called when no
                                       // handle task is running
  uint8_t loopCount = 10;
  while (loopCount && ((getRxMsgsCount() > 0) || (getTxMsgsCount() > 0))) {
    esp3d_log("flushing Tx messages");
    loopCount--;
    _handle_msgs();
  }
}

void ESP3DGCodeHostService::end() {
  if (_started) {
    flush();
    _started = false;
    esp3d_log("Clearing queue Rx messages");
    clearRxQueue();
    esp3d_log("Clearing queue Tx messages");
    clearTxQueue();
    esp3d_hal::wait(1000);
    while (!_scripts.empty()) {
      _popFrontGCodeStream(false);
    }
    while (!_streams.empty()) {
      _popFrontGCodeStream(true);
    }

    if (pthread_mutex_destroy(&_tx_mutex) != 0) {
      esp3d_log("Mutex destruction for tx failed");
    }
    if (pthread_mutex_destroy(&_rx_mutex) != 0) {
      esp3d_log("Mutex destruction for rx failed");
    }
    if (pthread_mutex_destroy(&_streams_list_mutex) != 0) {
      esp3d_log("Mutex destruction for streams list failed");
    }
    if (pthread_mutex_destroy(&_scripts_list_mutex) != 0) {
      esp3d_log("Mutex destruction for scripts list failed");
    }
  }
  if (_xHandle) {
    vTaskDelete(_xHandle);
    _xHandle = NULL;
  }

  if (_file_handle) {
    _closeFile(_current_main_stream_ptr);
    _file_handle = nullptr;
    // NOTE
    //  it won't never be used because by the destructor, because it will never
    //  be called to stop the class
    // but end is called when the class is started to clear all variables
    // so no need to release the FS actually
    // globalFs.releaseFS("/sd/");
    // globalFs.releaseFS("/fs/");
  }
  _current_main_stream_ptr = nullptr;
  _current_stream_ptr = nullptr;
  _error = ESP3DGcodeHostError::no_error;
  _requested_state = ESP3DGcodeStreamState::undefined;
}
