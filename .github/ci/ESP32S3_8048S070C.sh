#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
cd $GITHUB_WORKSPACE/esp-idf
source ./export.sh
cd $GITHUB_WORKSPACE
idf.py fullclean
sed -i '1,36s/ ON/ OFF/g' ./CMakeLists.txt
sed -i '1,36s/ESP32S3_8048S070C "TFT TARGET is ESP32S3_8048S070C - 7.0in. 800x480 (Capacitive)" OFF/ESP32S3_8048S070C "TFT TARGET is ESP32S3_8048S070C - 7.0in. 800x480 (Capacitive)" ON/g' ./CMakeLists.txt
idf.py fullclean
head ./CMakeLists.txt -n 9
idf.py -DIDF_TARGET=esp32s3 reconfigure
idf.py build
