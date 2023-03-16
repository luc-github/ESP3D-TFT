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

#pragma once
#include <stdio.h>

#include <list>
#include <string>

#include "esp3d_authentication_records.h"
#include "esp3d_authentication_types.h"
#include "esp3d_log.h"
#include "lwip/sockets.h"

#ifdef __cplusplus
extern "C" {
#endif

class ESP3DAuthenticationService final {
 public:
  ESP3DAuthenticationService();
  ~ESP3DAuthenticationService();
  ESP3DAuthenticationLevel getAuthenticatedLevel(const char *pwd = nullptr);
  bool begin();
  void handle();
  void end();
  bool isAdmin(const char *pwd);
  bool isUser(const char *pwd);
  void updateRecords();
#if ESP3D_AUTHENTICATION_FEATURE
  const char *getAdminPassword() { return _admin_pwd.c_str(); }
  const char *getUserPassword() { return _user_pwd.c_str(); }
  void setAdminPassword(const char *pwd) { _admin_pwd = pwd; }
  void setUserPassword(const char *pwd) { _user_pwd = pwd; }
  void setSessionTimeout(uint8_t timeout)  // minutes
  {
    _session_timeout = timeout;
  }
  uint64_t getSessionTimeout()  // milliseconds

  {
    return 60 * 1000 * _session_timeout;
  }
  bool createRecord(const char *sessionId, int socketId,
                    ESP3DAuthenticationLevel level,
                    ESP3DClientType client_type);
  bool clearSession(const char *sessionId);
  void clearSessions(ESP3DClientType client_type);
  bool updateRecord(int socketId, ESP3DClientType client_type,
                    ESP3DAuthenticationLevel newlevel);
  void clearAllSessions();
  ESP3DAuthenticationRecord *getRecord(const char *sessionId);
  ESP3DAuthenticationRecord *getRecord(int socketId,
                                       ESP3DClientType client_type);
  const char *create_session_id(struct sockaddr_storage source_addr,
                                int socketId);
  uint8_t activeSessionsCount(ESP3DClientType type);
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
 private:
#if ESP3D_AUTHENTICATION_FEATURE
  std::string _admin_pwd;
  std::string _user_pwd;
  uint8_t _session_timeout;
  std::list<ESP3DAuthenticationRecord> _sessions;
#endif  // #if ESP3D_AUTHENTICATION_FEATURE
};

extern ESP3DAuthenticationService esp3dAuthenthicationService;

#ifdef __cplusplus
}  // extern "C"
#endif