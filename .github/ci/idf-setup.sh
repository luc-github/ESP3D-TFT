#!/bin/bash
# Exit immediately if a command exits with a non-zero status.
set -e
git clone -b v5.1.1 --recursive --shallow-submodules https://github.com/espressif/esp-idf.git
cd ./esp-idf
./install.sh
source ./export.sh
cd ..
idf.py fullclean
