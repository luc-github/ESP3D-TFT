/*
  esp3d_commands member
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

#include <math.h>

#include <string>

#include "esp3d_client.h"
#include "esp3d_commands.h"
#include "esp3d_string.h"

#if ESP3D_TIMESTAMP_FEATURE
#include <time.h>
#endif  // ESP3D_TIMESTAMP_FEATURE
#include "authentication/esp3d_authentication.h"
#include "filesystem/esp3d_globalfs.h"
#define COMMAND_ID 780
// List Global Filesystem
///[ESP780]<Root> json=<no> pwd=<user/admin password>
void ESP3DCommands::ESP780(int cmd_params_pos, ESP3DMessage *msg) {
  bool hasError = false;
  std::string error_msg = "Path inccorrect";
  std::string ok_msg = "ok";
  bool json = hasTag(msg, cmd_params_pos, "json");
  std::string tmpstr;
  // prepare answer msg
  msg->target = msg->origin;
  msg->origin = ESP3DClientType::command;
#if ESP3D_AUTHENTICATION_FEATURE
  if (msg->authentication_level == ESP3DAuthenticationLevel::guest) {
    dispatchAuthenticationError(msg, COMMAND_ID, json);
    return;
  }
#endif  // ESP3D_AUTHENTICATION_FEATURE

  ESP3DMessage msgInfo;
  ESP3DClient::copyMsgInfos(&msgInfo, *msg);
  tmpstr = get_clean_param(msg, cmd_params_pos);

  if (tmpstr.length() == 0) {
    tmpstr = "/";
  }
  if (globalFs.accessFS(tmpstr.c_str())) {
    DIR *dir = globalFs.opendir(tmpstr.c_str());
    std::string currentPath;

    if (dir) {
      struct dirent *entry;
      struct stat entry_stat;
      if (json) {
        ok_msg = "{\"cmd\":\"780\",\"status\":\"ok\",\"data\":{\"path\":\"";
        ok_msg += tmpstr.c_str();
        ok_msg += "\",\"files\":[";

      } else {
        ok_msg = "Directory on Global Filesystem : ";
        ok_msg += tmpstr.c_str();
        ok_msg += "\n";
      }
      msg->type = ESP3DMessageType::head;
      if (!dispatch(msg, ok_msg.c_str())) {
        esp3d_log_e("Error sending response to clients");
      }
      uint nbDirs = 0;
      uint nbFiles = 0;
      uint64_t totalSpace = 0;
      uint64_t usedSpace = 0;
      uint64_t freeSpace = 0;
      globalFs.getSpaceInfo(&totalSpace, &usedSpace, &freeSpace, tmpstr.c_str(),
                            true);
      while ((entry = globalFs.readdir(dir)) != NULL) {
        currentPath = tmpstr + entry->d_name;
        if (entry->d_type == DT_DIR) {
          nbDirs++;
          if (json) {
            if (nbDirs || nbFiles) {
              ok_msg = ",";
            } else {
              ok_msg = "";
            }
            ok_msg += "{\"name\":\"";
            ok_msg += entry->d_name;
            ok_msg += "\",\"size\":\"-1\"}";
          } else {
            ok_msg = "[";
            ok_msg += entry->d_name;
            ok_msg += "]\n";
          }
        } else {
          continue;
        }
        ESP3DMessage *newMsg = ESP3DClient::copyMsgInfos(msgInfo);
        newMsg->type = ESP3DMessageType::core;
        if (!dispatch(newMsg, ok_msg.c_str())) {
          esp3d_log_e("Error sending response to clients");
        }
      }
      globalFs.rewinddir(dir);
      while ((entry = globalFs.readdir(dir)) != NULL) {
        currentPath = tmpstr;
        if (tmpstr[tmpstr.length() - 1] != '/') {
          currentPath += "/";
        }
        currentPath += entry->d_name;
        if (entry->d_type == DT_DIR) {
          continue;
        } else {
          nbFiles++;
          if (globalFs.stat(currentPath.c_str(), &entry_stat) == -1) {
            esp3d_log_e("Failed to stat %s : %s",
                        entry->d_type == DT_DIR ? "DIR" : "FILE",
                        entry->d_name);
            continue;
          }
#if ESP3D_TIMESTAMP_FEATURE
          char buff[20];
          strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S",
                   localtime(&(entry_stat.st_mtim.tv_sec)));
#endif  // ESP3D_TIMESTAMP_FEATURE
          if (json) {
            if (nbDirs || nbFiles) {
              ok_msg = ",";
            } else {
              ok_msg = "";
            }
            ok_msg += "{\"name\":\"";
            ok_msg += entry->d_name;
            ok_msg += "\",\"size\":\"";
            ok_msg += esp3d_string::formatBytes(entry_stat.st_size);
#if ESP3D_TIMESTAMP_FEATURE
            ok_msg += "\",\"time\":\"";
            ok_msg += buff;
#endif  // ESP3D_TIMESTAMP_FEATURE
            ok_msg += "\"}";
          } else {
            ok_msg = entry->d_name;
            // simple formating that should work 70% of the time
            if (strlen(entry->d_name) < 8) {
              ok_msg += "\t\t\t";
            } else if (strlen(entry->d_name) < 17) {
              ok_msg += "\t\t";
            } else {
              ok_msg += "\t";
            }
#if ESP3D_TIMESTAMP_FEATURE
            ok_msg += buff;
            ok_msg += "\t";
#endif  // ESP3D_TIMESTAMP_FEATURE
            ok_msg += esp3d_string::formatBytes(entry_stat.st_size);

            ok_msg += "\n";
          }
        }
        ESP3DMessage *newMsg = ESP3DClient::copyMsgInfos(msgInfo);
        newMsg->type = ESP3DMessageType::core;
        if (!dispatch(newMsg, ok_msg.c_str())) {
          esp3d_log_e("Error sending response to clients");
        }
      }
      if (json) {
        ok_msg = "], \"total\":\"";
        ok_msg += esp3d_string::formatBytes(totalSpace);
        ok_msg += "\",\"used\":\"";
        ok_msg += esp3d_string::formatBytes(usedSpace);
        ok_msg += "\",\"occupation\":\"";
        if (totalSpace == 0) {
          totalSpace = 1;
        }
        uint occupation = round(100.0 * usedSpace / totalSpace);
        if ((occupation < 1) && (usedSpace > 0)) {
          occupation = 1;
        }
        ok_msg += std::to_string(occupation);
        ok_msg += "\"}}";

      } else {
        ok_msg = "Files: ";
        ok_msg += std::to_string(nbFiles);
        ok_msg += ", Dirs :";
        ok_msg += std::to_string(nbDirs);
        ok_msg += "\nTotal: ";
        ok_msg += esp3d_string::formatBytes(totalSpace);
        ok_msg += ", Used: ";
        ok_msg += esp3d_string::formatBytes(usedSpace);
        ok_msg += ", Available: ";
        ok_msg += esp3d_string::formatBytes(freeSpace);
        ok_msg += "\n";
      }
      ESP3DMessage *newMsg = ESP3DClient::copyMsgInfos(msgInfo);
      newMsg->type = ESP3DMessageType::tail;
      if (!dispatch(newMsg, ok_msg.c_str())) {
        esp3d_log_e("Error sending response to clients");
      }
      globalFs.closedir(dir);
    } else {
      hasError = true;
      error_msg = "Cannot open :";
      error_msg += tmpstr.c_str();
      esp3d_log_e("%s", error_msg.c_str());
      if (!dispatchAnswer(msg, COMMAND_ID, json, hasError,
                          hasError ? error_msg.c_str() : ok_msg.c_str())) {
        esp3d_log_e("Error sending response to clients");
      }
    }
    globalFs.releaseFS(tmpstr.c_str());

  } else {
    hasError = true;
    error_msg = "Filesystem not available";
    esp3d_log_e("%s", error_msg.c_str());
    if (!dispatchAnswer(msg, COMMAND_ID, json, true,
                        hasError ? error_msg.c_str() : ok_msg.c_str())) {
      { esp3d_log_e("Error sending response to clients"); }
    }
  }
}