/*
  esp3d_authentication
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

#include "esp3d_authentication.h"

#include <stdio.h>

#include "esp3d_hal.h"
#include "esp3d_settings.h"

ESP3DAuthenticationService esp3dAuthenthicationService;

ESP3DAuthenticationService::ESP3DAuthenticationService() {}
ESP3DAuthenticationService::~ESP3DAuthenticationService() {}
ESP3DAuthenticationLevel ESP3DAuthenticationService::getAuthenticatedLevel(
    const char *pwd) {
#if ESP3D_AUTHENTICATION_FEATURE
  if (isAdmin(pwd)) {
    return ESP3DAuthenticationLevel::admin;
  }
  if (isUser(pwd)) {
    return ESP3DAuthenticationLevel::user;
  }
  return ESP3DAuthenticationLevel::guest;
#else
  return ESP3DAuthenticationLevel::admin;
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
}
bool ESP3DAuthenticationService::begin() {
#if ESP3D_AUTHENTICATION_FEATURE
  char tmpStr[SIZE_OF_LOCAL_PASSWORD + 1];
  _admin_pwd = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_admin_password, tmpStr, SIZE_OF_LOCAL_PASSWORD);
  _user_pwd = esp3dTftsettings.readString(
      ESP3DSettingIndex::esp3d_user_password, tmpStr, SIZE_OF_LOCAL_PASSWORD);
  _session_timeout =
      esp3dTftsettings.readByte(ESP3DSettingIndex::esp3d_session_timeout);
#endif  // ESP3D_AUTHENTICATION_FEATURE
  return true;
}
void ESP3DAuthenticationService::handle() {
  // TODO  clear time out session
}
void ESP3DAuthenticationService::end() {
#if ESP3D_AUTHENTICATION_FEATURE
  _admin_pwd.clear();
  _user_pwd.clear();
  clearAllSessions();
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
}
bool ESP3DAuthenticationService::isAdmin(const char *pwd) {
#if ESP3D_AUTHENTICATION_FEATURE
  esp3d_log("Checking %s with %s = %s", pwd, _admin_pwd.c_str(),
            _admin_pwd == pwd ? "true" : "false");
  return _admin_pwd == pwd;
#else
  return true;
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
}
bool ESP3DAuthenticationService::isUser(const char *pwd) {
#if ESP3D_AUTHENTICATION_FEATURE
  esp3d_log("Checking %s with %s = %s", pwd, _user_pwd.c_str(),
            _user_pwd == pwd ? "true" : "false");
  return _user_pwd == pwd;
#else
  return true;
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
}

#if ESP3D_AUTHENTICATION_FEATURE
const char *ESP3DAuthenticationService::create_session_id(
    struct sockaddr_storage source_addr, int socketId) {
  static char sessionID[25];
  memset(sessionID, 0, sizeof(sessionID));
  // get time
  uint32_t now = esp3d_hal::millis();
  uint32_t ip = (uint32_t)((struct sockaddr_in *)&source_addr)->sin_addr.s_addr;
  // generate SESSIONID
  if (0 > sprintf(sessionID, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
                  (uint8_t)(ip & 0xFF), (uint8_t)((ip >> 8) & 0xFF),
                  (uint8_t)((ip >> 16) & 0xFF), (uint8_t)((ip >> 24) & 0xFF),
                  (uint8_t)(socketId & 0xFF), (uint8_t)((socketId >> 8) & 0xFF),
                  (uint8_t)((socketId >> 16) & 0xFF),
                  (uint8_t)((socketId >> 24) & 0xFF),
                  (uint8_t)((now >> 0) & 0xff), (uint8_t)((now >> 8) & 0xff),
                  (uint8_t)((now >> 16) & 0xff),
                  (uint8_t)((now >> 24) & 0xff))) {
    strcpy(sessionID, "NONE");
  }

  return sessionID;
}

bool ESP3DAuthenticationService::createRecord(const char *sessionId,
                                              int socketId,
                                              ESP3DAuthenticationLevel level,
                                              ESP3DClientType client_type) {
  if (strlen(sessionId) == 0) {
    return false;
  }
  ESP3DAuthenticationRecord rec;
  memset(&rec, 0, sizeof(rec));
  rec.level = level;
  rec.client_type = client_type;
  rec.socket_id = socketId;
  strcpy(rec.session_id, sessionId);
  rec.last_time = esp3d_hal::millis();
  _sessions.push_back(rec);
  return true;
}

bool ESP3DAuthenticationService::clearSession(const char *sessionId) {
  if (sessionId && strlen(sessionId) == 24) {
    esp3d_log("Clear session %s, size:%d  among %d sessions", sessionId,
              strlen(sessionId), _sessions.size());
    for (auto session = _sessions.begin(); session != _sessions.end();
         ++session) {
      esp3d_log("checking session %s vs %s, type: %d, socketId: %d", sessionId,
                session->session_id, static_cast<uint8_t>(session->client_type),
                session->socket_id);
      if (strcmp(session->session_id, sessionId) == 0) {
        esp3d_log("Clear session %s succeed", sessionId);
        _sessions.erase(session++);
        return true;
      }
    }
  } else {
    esp3d_log_w("Empty sessionId");
  }
  esp3d_log_w("Clear session failed");
  return false;
}

void ESP3DAuthenticationService::clearSessions(ESP3DClientType client_type) {
  esp3d_log("Clear all sessions %d", static_cast<uint8_t>(client_type));
  for (auto session = _sessions.begin(); session != _sessions.end();
       ++session) {
    if (session->client_type == client_type) {
      esp3d_log("Clear session %s succeed", session->session_id);
      _sessions.erase(session++);
    }
  }
}

void ESP3DAuthenticationService::clearAllSessions() { _sessions.clear(); }
ESP3DAuthenticationRecord *ESP3DAuthenticationService::getRecord(
    const char *sessionId) {
  for (auto session = _sessions.begin(); session != _sessions.end();
       ++session) {
    if (strcmp(session->session_id, sessionId) == 0) {
      return &(*session);
    }
  }
  return NULL;
}

ESP3DAuthenticationRecord *ESP3DAuthenticationService::getRecord(
    int socketId, ESP3DClientType client_type) {
  for (auto session = _sessions.begin(); session != _sessions.end();
       ++session) {
    if ((socketId == -1 || session->socket_id == socketId) &&
        session->client_type == client_type) {
      esp3d_log("Found session %s", session->session_id);
      return &(*session);
    }
  }
  esp3d_log("Socket session not found");
  return NULL;
}

bool ESP3DAuthenticationService::updateRecord(
    int socketId, ESP3DClientType client_type,
    ESP3DAuthenticationLevel newlevel) {
  for (auto session = _sessions.begin(); session != _sessions.end();
       ++session) {
    if (session->client_type == client_type && session->socket_id == socketId) {
      session->level = newlevel;
      return true;
    }
  }
  return false;
}

uint8_t ESP3DAuthenticationService::activeSessionsCount(ESP3DClientType type) {
  uint8_t count = 0;
  for (auto session = _sessions.begin(); session != _sessions.end();
       ++session) {
    if (session->client_type == type) {
      esp3d_log("Session found: %s, socket: %d, lvl: %d ", session->session_id,
                session->socket_id, static_cast<uint8_t>(session->level));
      ++count;
    }
  }
  return count;
}

void ESP3DAuthenticationService::updateRecords() {
  for (auto session = _sessions.begin(); session != _sessions.end();
       ++session) {
    esp3d_log("session %s, type %d", session->session_id,
              static_cast<uint8_t>(session->client_type));
    // todo if type == webui check timestamp and last update
    // if session timout=0 =>TBD
    // other : TBD
  }
}
#endif  // #if ESP3D_AUTHENTICATION_FEATURE