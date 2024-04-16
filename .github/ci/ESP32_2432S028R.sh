#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
cd $GITHUB_WORKSPACE/esp-idf
source ./export.sh
cd $GITHUB_WORKSPACE
idf.py fullclean
sed -i '1,32s/ ON/ OFF/g' ./CMakeLists.txt
sed -i '1,32s/OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R - 2.8in. 320x240 (Resistive)" OFF/OPTION(ESP32_2432S028R "TFT TARGET is ESP32_2432S028R - 2.8in. 320x240 (Resistive)" ON/g' ./CMakeLists.txt
idf.py fullclean
head ./CMakeLists.txt -n 9
idf.py -DIDF_TARGET=esp32 reconfigure
idf.py build
