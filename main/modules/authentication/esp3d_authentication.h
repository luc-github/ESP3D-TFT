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
#include <string>
#include "esp3d_log.h"
#include "lwip/sockets.h"
#include "esp3d_authentication_types.h"
#include "esp3d_authentication_records.h"
#include <list>

#ifdef __cplusplus
extern "C" {
#endif

class Esp3DAuthenticationService final
{
public:
    Esp3DAuthenticationService();
    ~Esp3DAuthenticationService();
    esp3d_authentication_level_t  getAuthenticatedLevel(const  char * pwd = nullptr);
    bool begin();
    void handle();
    void end();
    bool isadmin (const char *pwd);
    bool isuser (const char *pwd);
    void updateRecords();
#if ESP3D_AUTHENTICATION_FEATURE
    void setAdminPassword( const char *pwd)
    {
        _admin_pwd = pwd;
    }
    void setUserPassword( const char *pwd)
    {
        _user_pwd = pwd;
    }
    void setSessionTimeout(uint8_t timeout)
    {
        _session_timeout = timeout;
    }
    bool createRecord (const char * sessionId, int socketId, esp3d_authentication_level_t level, esp3d_clients_t client_type);
    bool clearSession(const char * sessionId);
    bool updateRecord(int socketId, esp3d_clients_t client_type, esp3d_authentication_level_t newlevel);
    void clearAllSessions();
    esp3d_authentication_record_t * getRecord(const char * sessionId);
    const char* create_session_id(struct sockaddr_storage source_addr, int socketId);
#endif //#if ESP3D_AUTHENTICATION_FEATURE
private:

    bool _is_admin;
    bool _is_user;
#if ESP3D_AUTHENTICATION_FEATURE
    std::string _admin_pwd;
    std::string _user_pwd;
    uint8_t _session_timeout;
    std::list<esp3d_authentication_record_t> _sessions;
#endif //#if ESP3D_AUTHENTICATION_FEATURE
};

extern Esp3DAuthenticationService esp3dAuthenthicationService;

#ifdef __cplusplus
} // extern "C"
#endif