#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
cd $GITHUB_WORKSPACE/esp-idf
source ./export.sh
cd $GITHUB_WORKSPACE
sed -i '1,32s/ ON/ OFF/g' ./CMakeLists.txt
sed -i '1,32s/ESP32S3_8048S070 "TFT TARGET is ESP32S3 7.0inches" OFF/ESP32S3_8048S070 "TFT TARGET is ESP32S3 7.0inches" ON/g' ./CMakeLists.txt
idf.py fullclean
head ./CMakeLists.txt -n 9
idf.py -DIDF_TARGET=esp32s3 reconfigure
idf.py build
