# Project archiecture reference (Work in Progress)

## Introduction

The Goal of this document is to provide a high level overview of the project architecture. This document is a work in progress and will be updated as the project evolves.

## Files and Directories

The project is organized into the following directories:

- `components`: Contains the main components of the project
- `customization`: Contains the customization files for the project to avoid to change in sources
- `docs`: Contains project documentation
- `embedded`: Contains the code for embedded maintenance web page of the firmware
- `hardware`: Contains the hardware specific files like drivers, partitions description, specific sdkconfig, hardware configuration files
- `resources`: Contains the graphical resources like logo for bootsplash, icons, etc
- `main`: Contains the main code of the project   
- `scripts`: Contains the scripts used for the project like the fonts builder, the language pack builder, etc
- `tools`: Contains the tools used for the project to test features 
- `translations`: Contains the language files for the firmware
- `CMakelists.txt`: The main CMake file for the project
- `LICENSE`: The license file for the project
- `README.md`: The main readme file for the project
- `.gitignore`: The git ignore file for the project
- `.gitmodules`: The git submodules file for the project
- `.github`: The github actions directory for the project

### components directory

The `components` directory contains the main components of the project. The components are organized into the following subdirectories:

- `esp_litlefs`: Contains the code for the LittleFS file system
- `esp3d_log`: Contains the code for the logging system
- `lvgl`: Contains the code for the LittlevGL graphics library
- `mdns`: Contains the code for the mDNS service
- `SSDP_IDF`: Contains the code for the SSDP service

Note SSDP_IDF and esp_littlefs are actually defined as git submodules.

### customizations directory

The `customizations` directory contains the customization files for the project. The customization files are organized into the following subdirectories:
- `notifications`: Contains the customization strings for the notifications
- `ssdp`: Contains the customization strings for the SSDP service

### docs directory


#### embedded directory

The `embedded` directory contains the code for the embedded maintenance web page of the firmware. The code is organized into the following subdirectories:
- `assets`: Contains the assets for the embedded maintenance web page
    * `favicon.ico`: The favicon for the embedded maintenance web page
    * `header.txt`: The header for the embedded maintenance web page
    * `footer.txt`: The footer for the embedded maintenance web page

- `config`: Contains the configuration files for the test server and production page
    * `buildassets.js`: This script will convert the binaries assets into a C header file
    * `server.js`: The test server for the embedded maintenance web page
    * `webpack.dev.js`: The webpack configuration for the test server and test page
    * `webpack.prod.js`: The webpack configuration for the production page
- `dist`: Contains the binary distribution file for the embedded maintenance web page
- `server`: Is the directory that simulate the local flash system for the upload
- `src`: Contains the source code for the embedded maintenance web page
    * `index.html`: The main html file for the embedded maintenance web page
    * `index.js`: The main javascript file for the embedded maintenance web page
    * `menu.js`: The menu javascript file for the embedded maintenance web page
    * `style.css`: The main css file for the embedded maintenance web page
- `package.json`: The package file for the embedded maintenance web page
- `Notes.txt`: The notes file for the embedded maintenance web page
- `.gitignore`: The git ignore file for the embedded maintenance web page


#### hardware directory

The `hardware` directory contains the hardware specific files like drivers, partitions description, specific sdkconfig, hardware configuration files. The hardware specific files are organized into one common subdirectory and one subdirectory for each supported hardware platform.


| Drivers | depend | ESP32_2432S028R |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35| ESP32_CUSTOM |
|---|---|:---:|:--:|:--:|:--:|:--:|
|disp_backlight|esp3d_log driver| X | X | X | O | O|
|disp_spi| esp3d_log driver | O |  O  | O | X | O |
|ili9341|esp3d_log esp_lcd driver| X |  O | O | O | O |
|ili9488| esp3d_log lvgl disp_spi | O |  O | O | X | O |
|st7796|esp3d_log esp_lcd driver| O |  X | X | O | O |
|rm68120|esp3d_log lvgl esp_lcd driver| O |  O | O | O| O |
|xpt2046|esp3d_log driver| X |  O | X | X| O |
|gt911|esp3d_log i2c_bus| O | X | O | O| O |
|ft5x06|esp3d_log lvgl i2c_bus| O |  O | O | O| O
|tca9554|esp3d_log i2c_bus| O |  O | O | O| O |
|i2c_bus|esp3d_log driver| O | X | O| O| O |
|spi_bus|esp3d_log driver| X |  X | X | X| O
|sw_spi|esp3d_log driver| sw_spi |  O | O | O| O
|partition|| ESP32_2432S028R |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35 | ESP32_CUSTOM
|bsp|| ESP32_2432S028R |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35 | ESP32_CUSTOM


| Drivers | depend | ESP32S3_4827S043C | ESP32S3_8048S043C | ESP32S3_8048S050C | ESP32S3_8048S070C | ESP32S3_BZM_TFT35_GT911 | ESP32S3_HMI43V3 | ESP32S3_ZX3D50CE02S_USRC_4832 | ESP32S3_CUSTOM|
|---|---|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
|disp_backlight|esp3d_log driver| X | X | X | X | X | O | O | O |
|disp_spi| esp3d_log driver | O | O | O | O | O | O | O | O |
|ili9341|esp3d_log esp_lcd driver| O | O | O | O | O | O | O | O |
|ili9488|esp3d_log lvgl disp_spi| O | O | O | O | O | O | O | O |
|st7796|esp3d_log esp_lcd driver| O | O | O | O | X | O | X | O|
|rm68120|esp3d_log lvgl esp_lcd driver| O | O | O | O | O | X | O | O |
|xpt2046|esp3d_log driver| O | O | O | O | O | O | O | O|
|ft5x06|esp3d_log lvgl i2c_bus| O | O | O| O | O | X | X | O|
|gt911|esp3d_log i2c_bus| X | X | X | X | X | O | O | O |
|tca9554|esp3d_log i2c_bus| O | O | O | O | O | X | O | O|
|i2c_bus|esp3d_log driver| X | X | X | X | X | X | X | O|
|spi_bus|esp3d_log driver|  O | O | O | O | O | O | O | O|
|sw_spi|esp3d_log driver| O | O | O | O | O | O | O | O |
|usb_serial|esp3d_log| O | O | O | O | X | X | X | X |
|partition|| ESP32S3_4827S043C | ESP32S3_8048S043C | ESP32S3_8048S050C | ESP32S3_8048S070C | ESP32S3_BZM_TFT35_GT911 | ESP32S3_HMI43V3 | ESP32S3_ZX3D50CE02S_USRC_4832 | ESP32S3_CUSTOM|
|bsp|| ESP32S3_4827S043C | ESP32S3_8048S043C | ESP32S3_8048S050C | ESP32S3_8048S070C | ESP32S3_BZM_TFT35_GT911 | ESP32S3_HMI43V3 | ESP32S3_ZX3D50CE02S_USRC_4832 | ESP32S3_CUSTOM|