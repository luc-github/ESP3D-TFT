/*
  esp3d_config_file

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

#include <functional>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

typedef std::function<bool(const char *, const char *, const char *)>
    processingFunction_t;

class Esp3dConfigFile final {
 public:
  Esp3dConfigFile(const char *path, processingFunction_t fn,
                  const char *scrambledpath = nullptr,
                  const char **protectedkeys = nullptr);
  ~Esp3dConfigFile();
  char *trimSpaces(char *line, uint8_t maxsize = 0);
  bool isComment(char *line);
  bool isSection(char *line);
  bool isValue(char *line);
  char *getSectionName(char *line);
  char *getKeyName(char *line);
  char *getValue(char *line);
  bool processFile();
  bool revokeFile();

 private:
  bool isScrambleKey(const char *key, const char *str);
  std::string _filename;
  std::string _scrambledFilename;
  const char **_protectedkeys;
  processingFunction_t _pfunction;
};

#ifdef __cplusplus
}  // extern "C"
#endif