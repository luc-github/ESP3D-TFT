#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
cd $GITHUB_WORKSPACE/esp-idf
source ./export.sh
cd $GITHUB_WORKSPACE
idf.py fullclean
sed -i '1,36s/ ON/ OFF/g' ./CMakeLists.txt
sed -i '1,36s/OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" OFF/OPTION(ESP32_ROTRICS_DEXARM35 "TFT TARGET is ESP32 Rotrics DexArm 3.5inches" ON/g' ./CMakeLists.txt
idf.py fullclean
head ./CMakeLists.txt -n 9
idf.py -DIDF_TARGET=esp32 reconfigure
idf.py build
