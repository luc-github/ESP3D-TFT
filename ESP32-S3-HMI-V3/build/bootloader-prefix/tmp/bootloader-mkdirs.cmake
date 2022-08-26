# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/luc/Documents/esp-idf/components/bootloader/subproject"
  "C:/Users/luc/Documents/GitHub/ESP3D-TFT/ESP32-S3-HMI-V3/build/bootloader"
  "C:/Users/luc/Documents/GitHub/ESP3D-TFT/ESP32-S3-HMI-V3/build/bootloader-prefix"
  "C:/Users/luc/Documents/GitHub/ESP3D-TFT/ESP32-S3-HMI-V3/build/bootloader-prefix/tmp"
  "C:/Users/luc/Documents/GitHub/ESP3D-TFT/ESP32-S3-HMI-V3/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/luc/Documents/GitHub/ESP3D-TFT/ESP32-S3-HMI-V3/build/bootloader-prefix/src"
  "C:/Users/luc/Documents/GitHub/ESP3D-TFT/ESP32-S3-HMI-V3/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/luc/Documents/GitHub/ESP3D-TFT/ESP32-S3-HMI-V3/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
