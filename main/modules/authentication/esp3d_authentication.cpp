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
#include "esp3d_settings.h"
Esp3DAuthenticationService esp3dAuthenthicationService;

Esp3DAuthenticationService::Esp3DAuthenticationService() {}
Esp3DAuthenticationService::~Esp3DAuthenticationService() {}
esp3d_authentication_level_t  Esp3DAuthenticationService::getAuthenticatedLevel(const  char * pwd )
{
    //TODO check if password is valid
    return ESP3D_LEVEL_ADMIN;
}
bool Esp3DAuthenticationService::begin()
{
#if ESP3D_AUTHENTICATION_FEATURE
    char tmpStr [SIZE_OF_LOCAL_PASSWORD+1];
    _admin_pwd = esp3dTFTsettings.readString(esp3d_admin_password,tmpStr, SIZE_OF_LOCAL_PASSWORD);
    _user_pwd = esp3dTFTsettings.readString(esp3d_user_password,tmpStr, SIZE_OF_LOCAL_PASSWORD);
    _session_timeout = esp3dTFTsettings.readByte(esp3d_session_timeout);
#endif //ESP3D_AUTHENTICATION_FEATURE
    return true;
}
void Esp3DAuthenticationService::handle()
{
    //TODO  clear time out session
}
void Esp3DAuthenticationService::end()
{
    _admin_pwd.clear();
    _user_pwd.clear();

}
bool Esp3DAuthenticationService::isadmin (const char *pwd)
{
#if ESP3D_AUTHENTICATION_FEATURE
    return _admin_pwd==pwd;
#else
    return true;
#endif //#if ESP3D_AUTHENTICATION_FEATURE
}
bool Esp3DAuthenticationService::isuser (const char *pwd)
{
#if ESP3D_AUTHENTICATION_FEATURE
    return _user_pwd==pwd;
#else
    return true;
#endif //#if ESP3D_AUTHENTICATION_FEATURE
}
void Esp3DAuthenticationService::update()
{
    //TODO update passwords from settings
}