#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
cd $GITHUB_WORKSPACE/esp-idf
source ./export.sh
cd $GITHUB_WORKSPACE
idf.py fullclean
sed -i '1,32s/ ON/ OFF/g' ./CMakeLists.txt
sed -i '1,32s/OPTION(ESP32S3_FREENOVE_1_1 "HARDWARE TARGET is ESP32S3 Freenove v1.1" OFF/OPTION(ESP32S3_FREENOVE_1_1 "HARDWARE TARGET is ESP32S3 Freenove v1.1" ON/g' ./CMakeLists.txt
sed -i 's/OPTION(CAMERA_SERVICE "Camera service" OFF/OPTION(CAMERA_SERVICE "Camera service" ON/g' ./CMakeLists.txt
sed -i 's/OPTION(TFT_UI_SERVICE "TFT UI service" ON/OPTION(TFT_UI_SERVICE "TFT UI service" OFF/g' ./CMakeLists.txt
idf.py fullclean
head ./CMakeLists.txt -n 9
idf.py -DIDF_TARGET=esp32s3 reconfigure
idf.py build
