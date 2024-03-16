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
|st7796|SPI Display|esp3d_log esp_lcd driver| O |  X | X | O | O |
|rm68120||esp3d_log lvgl esp_lcd driver| O |  O | O | O| O |
|xpt2046|SPI Touch|esp3d_log driver| X |  O | X | X| O |
|gt911||esp3d_log i2c_bus| O | X | O | O| O |
|ft5x06||esp3d_log lvgl i2c_bus| O |  O | O | O| O
|tca9554||esp3d_log i2c_bus| O |  O | O | O| O |
|i2c_bus||esp3d_log driver| O | X | O| O| O |
|spi_bus|SPI Bus|esp3d_log driver| X |  X | X | X| O
|sw_spi|Software SPI|esp3d_log driver| X |  O | O | O| O
|partition||| 4MB |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35 | ESP32_CUSTOM
|bsp||| ESP32_2432S028R |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35 | ESP32_CUSTOM


| Drivers |Type| Depend | ESP32S3_4827S043C | ESP32S3_8048S043C | ESP32S3_8048S050C | ESP32S3_8048S070C | ESP32S3_BZM_TFT35_GT911 | ESP32S3_HMI43V3 | ESP32S3_ZX3D50CE02S_USRC_4832 | ESP32S3_CUSTOM|
|---|---|---|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
|disp_backlight|Display component|esp3d_log driver| X | X | X | X | X | O | O | O |
|disp_spi|| esp3d_log driver | O | O | O | O | O | O | O | O |
|ili9341|SPI Display|esp3d_log esp_lcd driver| O | O | O | O | O | O | O | O |
|ili9488||esp3d_log lvgl disp_spi| O | O | O | O | O | O | O | O |
|st7796| SPI Display |esp3d_log esp_lcd driver| O | O | O | O | X | O | X | O|
|st7262/ILI9485 |RGB Display|esp3d_log esp_lcd driver| X | O | O | O | X | O | O | O|
|rm68120||esp3d_log lvgl esp_lcd driver| O | O | O | O | O | X | O | O |
|xpt2046|SPI Touch|esp3d_log driver| O | O | O | O | O | O | O | O|
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
 |ESP32_2432S028R | Ok |
 |ESP32_3248S035C | Ok |  
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
|st7796|Ok|
|st7262|Ok|
|rm68120| |
|xpt2046|Ok|
|ft5x06||
|gt911|Ok|
|tca9554||
|i2c_bus|Ok|
|spi_bus|Ok|
|sw_spi| Ok |
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

#### st7262 driver
The `st7262` driver is a RGB display driver that is used to control the ST7262 display. The `st7262` driver configuration is part of display driver configuration.

in disp_def.h:
```cpp
// Display orientation
/*
PORTRAIT                0
PORTRAIT_INVERTED       1
LANDSCAPE               2
LANDSCAPE_INVERTED      3
*/
#define DISP_ORIENTATION 2  // landscape

// Display resolution
#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
#define DISP_HOR_RES_MAX 480
#define DISP_VER_RES_MAX 272
#else  // portrait mode
#define DISP_HOR_RES_MAX 272
#define DISP_VER_RES_MAX 480
#endif

// Display interface
#define DISP_CLK_FREQ           (14 * 1000 * 1000)
#define DISP_AVOID_TEAR_EFFECT_WITH_SEM (true)
#define DISP_USE_BOUNCE_BUFFER  (false)
#define DISP_USE_DOUBLE_BUFFER  (true)
#define DISP_NUM_FB             (1)

#define DISP_PATCH_FS_FREQ (6 * 1000 * 1000)  // 6MHz
#define DISP_PATCH_FS_DELAY  (40)

#if DISP_NUM_FB == 2
  // Full frame buffer (255KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX)
#else
  // 1/4 (68-line) buffer (63.75KB) in external PSRAM
  #define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 4)
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES    (DISP_BUF_SIZE * 2)

// Display panel configuration
const esp_lcd_rgb_panel_config_t disp_panel_cfg = {
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .timings = {
        .pclk_hz = DISP_CLK_FREQ,
        .h_res = DISP_HOR_RES_MAX,
        .v_res = DISP_VER_RES_MAX,
        .hsync_pulse_width = 4,
        .hsync_back_porch = 8,
        .hsync_front_porch = 8,
        .vsync_pulse_width = 4,
        .vsync_back_porch = 8,
        .vsync_front_porch = 8,
        .flags = {
            .hsync_idle_low = 0,
            .vsync_idle_low = 0,
            .de_idle_high = 0,
            .pclk_active_neg = 1,
            .pclk_idle_high = 0,
        },
    },
    .data_width = 16, // RGB565 in parallel mode
    .bits_per_pixel = 0,
    .num_fbs = DISP_NUM_FB,
#if DISP_USE_BOUNCE_BUFFER
    .bounce_buffer_size_px = DISP_BUF_SIZE,
#else
    .bounce_buffer_size_px = 0,
#endif
    .sram_trans_align = 0,
    .psram_trans_align = 64,
    .hsync_gpio_num = 39, // GPIO 39
    .vsync_gpio_num = 41, // GPIO 41
    .de_gpio_num = 40, // GPIO 40
    .pclk_gpio_num = 42, // GPIO 42
    .disp_gpio_num = -1, // EN pin not connected
    .data_gpio_nums = {
        8,  // D0  (B0) - GPIO 8
        3,  // D1  (B1) - GPIO 3
        46, // D2  (B2) - GPIO 46
        9,  // D3  (B3) - GPIO 9
        1,  // D4  (B4) - GPIO 1
        5,  // D5  (G0) - GPIO 5
        6,  // D6  (G1) - GPIO 6
        7,  // D7  (G2) - GPIO 7
        15, // D8  (G3) - GPIO 15
        16, // D9  (G4) - GPIO 16
        4,  // D10 (G5) - GPIO 4
        45, // D11 (R0) - GPIO 45
        48, // D12 (R1) - GPIO 48
        47, // D13 (R2) - GPIO 47
        21, // D14 (R3) - GPIO 21
        14, // D15 (R4) - GPIO 14
    },
    .flags = {
        .disp_active_low = 0,
        .refresh_on_demand = 0,
        .fb_in_psram = 1, // Do not change this, as it is mandatory for RGB parallel interface and octal PSRAM
        .double_fb = 0,
        .no_fb = 0,
        .bb_invalidate_cache = 0,
    }
};
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

#### sw_spi driver
The `sw_spi` driver is a software SPI driver that is used to control the SPI bus. The `sw_spi` driver configuration is part of the driver configuration file which use it.

eg: touch driver use it, so it can be part of the touch driver configuration.   

touch_def.h:
```cpp
// SPI (BitBang)
const sw_spi_config_t touch_spi_cfg = {
    .cs_pin = 33,   // GPIO 33
    .clk_pin = 25,  // GPIO 25
    .mosi_pin = 32, // GPIO 33
    .miso_pin = 39, // GPIO 39
};
```


#### xpt2046 driver
The `xpt2046` driver is a touch driver that is used to control the XPT2046 touch controller. The `xpt2046` driver configuration is part of the touch driver configuration.

touch_def.h:
```cpp
// X/Y Calibration Values
#define TOUCH_X_MIN           380
#define TOUCH_Y_MIN           200
#define TOUCH_X_MAX           3950
#define TOUCH_Y_MAX           3850

xpt2046_config_t xpt2046_cfg = {
    .irq_pin = 36, // GPIO 36  
    .touch_threshold = 300, // Threshold for touch detection  
    .swap_xy = true,
    .invert_x = true,
    .invert_y = true,
};
```

#### i2c_bus driver
The `i2c_bus` driver is a I2C bus driver that is used to control the I2C bus. The `i2c_bus` driver configuration file is i2c_def.h.

bsp.c:
```cpp
//define the I2C bus 
#define I2C_PORT_NUMBER   0

// I2C pins definition
const i2c_config_t i2c_cfg = {
    .mode = I2C_MODE_MASTER,
    .scl_io_num = 20, // GPIO 20
    .sda_io_num = 19, // GPIO 19
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = 400*1000
};

```

#### gt911 driver
The `gt911` driver is a touch driver that is used to control the GT911 touch controller. The `gt911` driver configuration is part of the touch driver configuration file : touch_def.h.

touch_def.h:
```cpp
// GT911 touch controller configuration
const gt911_config_t gt911_cfg = {
    .i2c_clk_speed = 400*1000,
    .i2c_addr = (uint8_t[]){0x5D, 0X14,0},  // Floating or mis-configured INT pin may cause GT911 to come up at address 0x14 instead of 0x5D, so check there as well.
    .rst_pin = 38, // GPIO 38
#if WITH_GT911_INT
    .int_pin = 18, // GPIO 18
#else
    .int_pin = -1, // INT pin not connected (by default)
#endif
    .swap_xy = false,
    .invert_x = false,
    .invert_y = false,    
};
```