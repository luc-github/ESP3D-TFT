#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
cd $GITHUB_WORKSPACE/esp-idf
source ./export.sh
cd $GITHUB_WORKSPACE
sed -i 's/OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R 2.8inches" ON/OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R 2.8inches" OFF/g' ./CMakeLists.txt
sed -i 's/OPTION(ESP32S3_HMI43V3 "TFT TARGET is ESP32S3 HMI 4.3inches" ON/OPTION(ESP32S3_HMI43V3 "TFT TARGET is ESP32S3 HMI 4.3inches" OFF/g' ./CMakeLists.txt
sed -i 's/OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" ON/OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" OFF/g' ./CMakeLists.txt
sed -i 's/ESP32S3_ZX3D50CE02S_USRC_4832 "TFT TARGET is ESP32S3 Panlee ZX3D50CE02S-SRC-4832 3.5inches" OFF/ESP32S3_ZX3D50CE02S_USRC_4832 "TFT TARGET is ESP32S3 Panlee ZX3D50CE02S-SRC-4832 3.5inches" ON/g' ./CMakeLists.txt
idf.py fullclean
head ./CMakeLists.txt -n 8
idf.py -DIDF_TARGET=esp32s3 reconfigure
idf.py build
