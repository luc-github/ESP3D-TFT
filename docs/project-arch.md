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


| Drivers |Type | Depend | ESP32_2432S028R |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35| ESP32_CUSTOM |
|---|---|---|:---:|:--:|:--:|:--:|:--:|
|disp_backlight|Display component |esp3d_log driver| X | X | X | O | O|
|disp_spi|| esp3d_log driver | O |  O  | O |X  | O |
|ili9341|SPI Display|esp3d_log esp_lcd driver| X |  O | O | O | O |
|ili9488|| esp3d_log lvgl disp_spi | O |  O | O | X | O |
|st7796||esp3d_log esp_lcd driver| O |  X | X | O | O |
|rm68120||esp3d_log lvgl esp_lcd driver| O |  O | O | O| O |
|xpt2046||esp3d_log driver| X |  O | X | X| O |
|gt911||esp3d_log i2c_bus| O | X | O | O| O |
|ft5x06||esp3d_log lvgl i2c_bus| O |  O | O | O| O
|tca9554||esp3d_log i2c_bus| O |  O | O | O| O |
|i2c_bus||esp3d_log driver| O | X | O| O| O |
|spi_bus|SPI Bus|esp3d_log driver| X |  X | X | X| O
|sw_spi||esp3d_log driver| X |  O | O | O| O
|partition||| 4MB |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35 | ESP32_CUSTOM
|bsp||| ESP32_2432S028R |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35 | ESP32_CUSTOM


| Drivers |Type| Depend | ESP32S3_4827S043C | ESP32S3_8048S043C | ESP32S3_8048S050C | ESP32S3_8048S070C | ESP32S3_BZM_TFT35_GT911 | ESP32S3_HMI43V3 | ESP32S3_ZX3D50CE02S_USRC_4832 | ESP32S3_CUSTOM|
|---|---|---|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
|disp_backlight|Display component|esp3d_log driver| X | X | X | X | X | O | O | O |
|disp_spi|| esp3d_log driver | O | O | O | O | O | O | O | O |
|ili9341|SPI Display|esp3d_log esp_lcd driver| O | O | O | O | O | O | O | O |
|ili9488||esp3d_log lvgl disp_spi| O | O | O | O | O | O | O | O |
|st7796||esp3d_log esp_lcd driver| O | O | O | O | X | O | X | O|
|rm68120||esp3d_log lvgl esp_lcd driver| O | O | O | O | O | X | O | O |
|xpt2046||esp3d_log driver| O | O | O | O | O | O | O | O|
|ft5x06||esp3d_log lvgl i2c_bus| O | O | O| O | O | X | X | O|
|gt911||esp3d_log i2c_bus| X | X | X | X | X | O | O | O |
|tca9554||esp3d_log i2c_bus| O | O | O | O | O | X | O | O|
|i2c_bus||esp3d_log driver| X | X | X | X | X | X | X | O|
|spi_bus|SPI Bus|esp3d_log driver|  O | O | O | O | O | O | O | O|
|sw_spi||esp3d_log driver| O | O | O | O | O | O | O | O |
|usb_serial||esp3d_log| O | O | O | O | X | X | X | X |
|partition||| ESP32S3_4827S043C | ESP32S3_8048S043C | ESP32S3_8048S050C | ESP32S3_8048S070C | ESP32S3_BZM_TFT35_GT911 | ESP32S3_HMI43V3 | ESP32S3_ZX3D50CE02S_USRC_4832 | ESP32S3_CUSTOM|
|bsp||| ESP32S3_4827S043C | ESP32S3_8048S043C | ESP32S3_8048S050C | ESP32S3_8048S070C | ESP32S3_BZM_TFT35_GT911 | ESP32S3_HMI43V3 | ESP32S3_ZX3D50CE02S_USRC_4832 | ESP32S3_CUSTOM|


### Refactored hardware directory
|Board| Status|
|---|:---:|
 |ESP32_2432S028R | On going |  
 |ESP32_3248S035C | 
 |ESP32_3248S035R | 
 |ESP32_ROTRICS_DEXARM35| 
 |ESP32_CUSTOM |
 |ESP32S3_4827S043C | 
 |ESP32S3_8048S043C | 
 |ESP32S3_8048S050C | 
 |ESP32S3_8048S070C | 
 |ESP32S3_BZM_TFT35_GT911 | 
 |ESP32S3_HMI43V3 | 
 |ESP32S3_ZX3D50CE02S_USRC_4832 | 
 |ESP32S3_CUSTOM|

|Drivers| Status|
|---|:---:|
|disp_backlight|Ok|
|disp_spi| |
|ili9341|Ok|
|ili9488| |
|st7796||
|rm68120| |
|xpt2046||
|ft5x06||
|gt911||
|tca9554||
|i2c_bus||
|spi_bus||
|sw_spi| On going  |
|usb_serial|| 


#### disp_backlight driver
The `disp_backlight` driver is a display component that is used by the display drivers to control the backlight of the display. The `disp_backlight` driver configuration is part of display driver configuration.


in disp_def.h:
```cpp
// Default backlight level value in percentage
#define DISP_BCKL_DEFAULT_DUTY 100  //%

// Backlight configuration
const disp_backlight_config_t disp_bcklt_cfg = {
    .pwm_control = false,   // true: LEDC is used, false: GPIO is used
    .output_invert = false, // true: LEDC output is inverted, false: LEDC output is not inverted
    .gpio_num = 21,         // GPIO number for backlight control
    // Relevant only for PWM controlled backlight
    // Ignored for switch (ON/OFF) backlight control
    .timer_idx = 0,         // LEDC timer index
    .channel_idx = 0        // LEDC channel index    
};
```

#### ili9341 driver
The `ili9341` driver is a SPI display driver that is used to control the ILI9341 display. The `ili9341` driver configuration is part of display driver configuration.

in disp_def.h:
```cpp

//Resolution according orientation
#define DISP_ORIENTATION 3  // landscape inverted

#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
#define DISP_HOR_RES_MAX 320
#define DISP_VER_RES_MAX 240
#else  // portrait mode
#define DISP_HOR_RES_MAX 240
#define DISP_VER_RES_MAX 320
#endif

// ILI9341 display driver configuration
esp_lcd_panel_dev_config_t disp_panel_cfg = {
    .reset_gpio_num = 4, // GPIO 4
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR, 
    .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
    .bits_per_pixel = 16,
    .flags = {
        .reset_active_high = 0
    },
    .vendor_config = NULL
};

//LVGL buffer definition depend if board has PSRAM or not
#define DISP_USE_DOUBLE_BUFFER (true)

#if WITH_PSRAM
  // 1/10 (24-line) buffer (15KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_SPIRAM
#else
  // 1/20 (12-line) buffer (7.5KB) in internal DRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 12)
  #define DISP_BUF_MALLOC_TYPE  MALLOC_CAP_DMA
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES    (DISP_BUF_SIZE * 2)
```

#### spi_bus driver
The `spi_bus` driver is a SPI bus driver that is used to control the SPI bus. The `spi_bus` driver configuration is part of display driver configuration.
in disp_def.h:
```cpp
// SPI (dedicated)
#define DISP_SPI_HOST SPI2_HOST  // 1

// SPI pins definition (common)
#define DISP_SPI_CLK  14  // GPIO 14
#define DISP_SPI_MOSI 13  // GPIO 13
#define DISP_SPI_MISO 12  // GPIO 12
```
