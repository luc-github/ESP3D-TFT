# #######################################
# Select the target
# #######################################
OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R 2.8inches" ON)
OPTION(ESP32S3_HMI43V3 "TFT TARGET is ESP32S3 HMI 4.3inches" OFF)
OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" OFF)
OPTION(ESP32S3_ZX3D50CE02S_USRC_4832 "TFT TARGET is ESP32S3 Panlee ZX3D50CE02S-USRC-4832 3.5inches" OFF)

# #######################################
# Do not change anything above this line
# #######################################
cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

if(ESP32_2432S028R)
    set(SDKCONFIG targets/ESP32_2432S028R/sdkconfig)
    add_compile_options(-DTFT_TARGET="ESP32_2432S028R")
    set(EXTRA_COMPONENT_DIRS targets/ESP32_2432S028R/components)
elseif(ESP32S3_HMI43V3)
    set(SDKCONFIG main/targets/ESP32S3_HMI43V3/sdkconfig)
    add_compile_options(-DTFT_TARGET="ESP32S3_HMI43V3")
    set(EXTRA_COMPONENT_DIRS main/targets/ESP32S3_HMI43V3/components)
elseif(ESP32_ROTRICS_DEXARM35)
    set(SDKCONFIG main/targets/ESP32_ROTRICS_DEXARM35/sdkconfig)
    add_compile_options(-DTFT_TARGET="ESP32_ROTRICS_DEXARM35")
    set(EXTRA_COMPONENT_DIRS main/targets/ESP32_ROTRICS_DEXARM35/components)
elseif(ESP32S3_ZX3D50CE02S_USRC_4832)
    set(SDKCONFIG main/targets/ESP32S3_ZX3D50CE02S_USRC_4832/sdkconfig)
    add_compile_options(-DTFT_TARGET="ESP32S3_ZX3D50CE02S_USRC_4832")
    set(EXTRA_COMPONENT_DIRS main/targets/ESP32S3_ZX3D50CE02S_USRC_4832/components)
else()
    message(FATAL_ERROR "No target defined, CMake will exit.")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP3D-TFT
    VERSION 1.0
    DESCRIPTION "ESP3D TFT")
