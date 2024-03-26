# Project archiecture reference (Work in Progress)

## Introduction

The Goal of this document is to provide a high level overview of the project architecture. This document is a work in progress and will be updated as the project evolves.

## Files and Directories

The project is organized into the following directories:

- `cmake`: Contains the cmake files for the project and for each target
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

### cmake directory

The `cmake` directory contains the cmake files for the project and for each target. The cmake files are organized into the following files and subdirectories:
- `dev_tools.cmake`: Contains the cmake file for the development tools (log level, benchmark, log color, etc), this file can be edited but only for debug purpose
- `features.cmake`: Contains the cmake file for the features of the project (wifi, webserver, etc)
- `sanity_check.cmake`: Contains the cmake file for the sanity check of the project
- `targets.cmake`: Complete the cmake file of the project according target board
- `targets` directory : contains the cmake file for each target board, the file is named as the target board name

Each target file contains the following sections:
```
# Ensure the board is enabled in CMakeLists.txt
if(<BOARD_NAME>) #<BOARD_NAME> is enabled or not in CMakeLists.txt
    # Add board name (Mandatory)
    set(TFT_TARGET "<BOARD_NAME>")
    # Add the sdkconfig file path (Mandatory), it can be specific or based on one of the common sdkconfig   
    set(SDKCONFIG  ${CMAKE_SOURCE_DIR}/hardware/<BOARD_NAME>/sdkconfig)
    # Add Specific Components if any for the board (Mandatory)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/<BOARD_NAME>/components)
    # Add specific usb driver for otg (Only if needed)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/drivers_usb_otg)
    # Add specific video driver for i80 (Only if needed)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/drivers_video_i80)
    # Add specific video driver for rgb (Only if needed)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/hardware/rgb)
    # Add specific bsp path for board definition (Mandatory)
    add_compile_options("-I${CMAKE_SOURCE_DIR}/hardware/<BOARD_NAME>/components/bsp")
    # Enable USB-OTG as serial alternative for communications (Only if needed)
    add_compile_options(-DESP3D_USB_SERIAL_FEATURE=1)
endif()
```

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
- `common`: Contains the common hardware specific files like  partitions description, specific sdkconfig
- `drivers_common`: Contains the common drivers for all the boards
- `drivers_video_i80`: Contains the drivers for the i80 video drivers which cannot be in common drivers because of the specific sdkconfig
- `drivers_video_rgb`: Contains the drivers for the rgb video drivers which cannot be in common drivers because of the specific sdkconfig
- `drivers_usb_otg`: Contains the drivers for the usb otg which cannot be in common drivers because of the specific sdkconfig and hardware
- `<BOARD_NAME>`: Contains the hardware drivers, definitions, partitions and sdkconfig specific files for the <BOARD_NAME> board
- ...


| Drivers |Type | Depend | ESP32_2432S028R |  ESP32_3248S035C | ESP32_3248S035R | ESP32_ROTRICS_DEXARM35| ESP32_CUSTOM |
|---|---|---|:---:|:--:|:--:|:--:|:--:|
|disp_backlight|Display component |esp3d_log driver| X | X | X | O | O|
|disp_spi|| esp3d_log driver | O |  O  | O |X  | O |
|ili9341|SPI Display|esp3d_log esp_lcd driver| X |  O | O | O | O |
|ili9488|| esp3d_log lvgl disp_spi | O |  O | O | X | O |
|st7796|SPI Display|esp3d_log esp_lcd driver| O |  X | X | O | O |
|xpt2046|SPI Touch|esp3d_log driver| X |  O | X | X| O |
|gt911||esp3d_log i2c_bus| O | X | O | O| O |
|ft5x06||esp3d_log lvgl i2c_bus| O |  O | O | O| O
|i2c_bus||esp3d_log driver| O | X | O| O| O |
|spi_bus|SPI Bus|esp3d_log driver| X |  X | X | X| O
|sw_spi|Software SPI|esp3d_log driver| X |  O | O | O| O



| Drivers |Type| Depend | ESP32S3_4827S043C | ESP32S3_8048S043C | ESP32S3_8048S050C | ESP32S3_8048S070C | ESP32S3_BZM_TFT35_GT911 | ESP32S3_HMI43V3 | ESP32S3_ZX3D50CE02S_USRC_4832 | ESP32S3_CUSTOM|
|---|---|---|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
|disp_backlight|Display component|esp3d_log driver| X | X | X | X | X | O | O | O |
|st7796| i80 Display |esp3d_log esp_lcd driver| O | O | O | O | X | O | X | O|
|ili9485 |RGB Display|esp3d_log esp_lcd driver| X | O | O | O | O | O | O | O|
|st7262 |RGB Display|esp3d_log esp_lcd driver| O | X | X | O | O | O | O | O|
|ek9716|RGB Display|esp3d_log lvgl esp_lcd driver| O | O | O | X| O | O | O | O |
|rm68120|i80 Display|esp3d_log lvgl esp_lcd driver| O | O | O | O | O | X | O | O |
|ft5x06|i2c Touch|esp3d_log lvgl i2c_bus| O | O | O| O | O | X | X | O|
|gt911|i2c Touch|esp3d_log i2c_bus| X | X | X | X | X | O | O | O |
|tca9554|IO expander|esp3d_log i2c_bus| O | O | O | O | O | X | O | O|
|i2c_bus|i2c Bus|esp3d_log driver| X | X | X | X | X | X | X | O|
|usb_serial| OTG Host|esp3d_log| O | O | O | O | X | X | X | X |



### Refactored hardware directory
|Board| Status|
|---|:---:|
 |ESP32_2432S028R | Ok |
 |ESP32_3248S035C | Ok |  
 |ESP32_3248S035R | Ok |
 |ESP32_ROTRICS_DEXARM35| 
 |ESP32_CUSTOM | Ok |
 |ESP32S3_4827S043C | Ok |
 |ESP32S3_8048S043C | Ok |
 |ESP32S3_8048S050C | Ok |
 |ESP32S3_8048S070C | Ok |
 |ESP32S3_BZM_TFT35_GT911 | Ok | 
 |ESP32S3_HMI43V3 |  Ok |
 |ESP32S3_ZX3D50CE02S_USRC_4832 | Ok |
 |ESP32S3_CUSTOM| Ok |
 |ESP32S3_FREENOVE_1_1| Ok |

|Drivers| Status|
|---|:---:|
|disp_backlight|Ok|
|disp_spi| |
|ili9341 SPI|Ok|
|ili9488 SPI| |
|st7796 SPI|Ok|
|st7796 i80|Ok|
|st7262 RGB|Ok|
|ili9485 RGB| Ok |
|ek9716 RGB||
|rm68120 i80|Ok |
|xpt2046 SPI|Ok|
|ft5x06 ic2|Ok|
|gt911 ic2 |Ok|
|tca9554|Ok|
|i2c_bus|Ok|
|spi_bus|Ok|
|sw_spi| Ok |
|usb_serial|Ok| 


### Bus drivers
* spi_bus driver
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

* sw_spi driver
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

* i2c_bus driver
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

* tca9554 driver
The `tca9554` driver is a I2C GPIO expander driver that is used to control the TCA9554 GPIO expander. The `tca9554` driver configuration file is tca9554_def.h.

tca9554_def.h:
```cpp
const tca9554_config_t tca9554_cfg = {.i2c_clk_speed = 400 * 1000,
                                      .i2c_addr = (uint8_t[]){0x3C, 0x24, 0}};
```

* usb_serial driver
The `usb_serial` driver is a USB serial driver that is used to control the OTG USB serial. The `usb_serial` driver configuration usb_serial_def.h.

usb_serial_def.h:
```cpp
#define ESP3D_USB_SERIAL_BAUDRATE "115200"
#define ESP3D_USB_SERIAL_DATA_BITS (8)
#define ESP3D_USB_SERIAL_PARITY \
  (0)  // 0: 1 stopbit, 1: 1.5 stopbits, 2: 2 stopbits
#define ESP3D_USB_SERIAL_STOP_BITS \
  (0)  // 0: None, 1: Odd, 2: Even, 3: Mark, 4: Space
  ```

### SPI Display drivers
* ili9341 driver
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

* st7796 spi driver
The `st7796` driver is a display driver that is used to control the ST7796 display. The `st7796` driver configuration is part of display driver configuration.

disp_def.h:
```cpp
// Display orientation
/*
PORTRAIT                0
PORTRAIT_INVERTED       1
LANDSCAPE               2
LANDSCAPE_INVERTED      3
*/
#define DISP_ORIENTATION 3  // landscape inverted

// Display resolution
#if DISP_ORIENTATION == 2 || DISP_ORIENTATION == 3  // landscape mode
#define DISP_HOR_RES_MAX 480
#define DISP_VER_RES_MAX 320
#else  // portrait mode
#define DISP_HOR_RES_MAX 320
#define DISP_VER_RES_MAX 480
#endif

// Display interface
#define DISP_USE_DOUBLE_BUFFER (true)

#if WITH_PSRAM
// 1/10 (32-line) buffer (30KB) in external PSRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * DISP_VER_RES_MAX / 10)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_SPIRAM
#else
// 1/40 (8-line) buffer (7.5KB) in internal DRAM
#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * 8)
#define DISP_BUF_MALLOC_TYPE MALLOC_CAP_DMA
#endif  // WITH_PSRAM
#define DISP_BUF_SIZE_BYTES (DISP_BUF_SIZE * 2)

// LCD panel configuration
esp_lcd_panel_dev_config_t disp_panel_cfg = {
    .reset_gpio_num = -1,  // st7796 RST not connected to GPIO
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
    .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
    .bits_per_pixel = 16,
    .flags = {.reset_active_high = 0},
    .vendor_config = NULL};

// SPI (dedicated)
#define DISP_SPI_HOST SPI2_HOST  // 1
// #define DISP_SPI_HOST SPI3_HOST //2

// SPI pins definition (common)
#define DISP_SPI_CLK 14   // GPIO 14
#define DISP_SPI_MOSI 13  // GPIO 13
#define DISP_SPI_MISO 12  // GPIO 12

// SPI definition for LCD
esp_lcd_panel_io_spi_config_t disp_spi_cfg = {
    .cs_gpio_num = 15, /*!< GPIO used for CS line */
    .dc_gpio_num = 2,  /*!< GPIO used to select the D/C line, set this to -1 if
                          the D/C line is not used */
    .spi_mode = 0,     /*!< Traditional SPI mode (0~3) */
    .pclk_hz = 40 * 1000 * 1000, /*!< Frequency of pixel clock */
    .trans_queue_depth = 10,     /*!< Size of internal transaction queue */
    .on_color_trans_done =
        NULL, /*!< Callback invoked when color data transfer has finished */
    .user_ctx = NULL,    /*!< User private data, passed directly to
                            on_color_trans_done's user_ctx */
    .lcd_cmd_bits = 8,   /*!< Bit-width of LCD command */
    .lcd_param_bits = 8, /*!< Bit-width of LCD parameter */
    .flags = {
        /*!< Extra flags to fine-tune the SPI device */
        .dc_low_on_data =
            0, /*!< If this flag is enabled, DC line = 0 means transfer data, DC
                  line = 1 means transfer command; vice versa */
        .octal_mode = 0, /*!< transmit with octal mode (8 data lines), this mode
                            is used to simulate Intel 8080 timing */
        .quad_mode = 0,  /*!< transmit with quad mode (4 data lines), this mode
                            is useful when transmitting LCD parameters (Only use
                            one line for command) */
        .sio_mode = 0,  /*!< Read and write through a single data line (MOSI) */
        .lsb_first = 0, /*!< transmit LSB bit first */
        .cs_high_active = 0, /*!< CS line is high active */
    }};
```

### RGB Display drivers
* st7262 driver
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

* i80 Display drivers

rm68120 driver
The `rm68120` driver is a display driver that is used to control the RM68120 display. The `rm68120` driver configuration is part of display driver configuration.

disp_def.h:
```cpp
#define DISP_ORIENTATION \
  (orientation_landscape)  // 0:portrait mode 1:landscape mode 2:portrait mode
                           // reverse 3:landscape mode reverse

#if DISP_ORIENTATION == orientation_landscape || \
    DISP_ORIENTATION == orientation_landscape_invert  // landscape mode
#define DISP_HOR_RES_MAX (800)
#define DISP_VER_RES_MAX (480)
#else  // portrait mode
#define DISP_HOR_RES_MAX (480)
#define DISP_VER_RES_MAX (800)
#endif

#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * (DISP_VER_RES_MAX / 10))
#define DISP_USE_DOUBLE_BUFFER (true)

const esp_i80_rm68120_config_t rm68120_cfg = {
    .bus_config =
        {
            .clk_src = LCD_CLK_SRC_DEFAULT,
            .dc_gpio_num = 38,  // DISP_RS_PIN=GPIO38
            .wr_gpio_num = 17,  // DISP_WR_PIN=GPIO17
            .data_gpio_nums =
                {
                    1,   // DISP_D00_PIN = GPIO1
                    9,   // DISP_D01_PIN = GPIO9
                    2,   // DISP_D02_PIN = GPIO2
                    10,  // DISP_D03_PIN = GPIO10
                    3,   // DISP_D04_PIN = GPIO3
                    11,  // DISP_D05_PIN = GPIO11
                    4,   // DISP_D06_PIN = GPIO4
                    12,  // DISP_D07_PIN = GPIO12
                    5,   // DISP_D08_PIN = GPIO5
                    13,  // DISP_D09_PIN = GPIO13
                    6,   // DISP_D10_PIN = GPIO6
                    14,  // DISP_D11_PIN = GPIO14
                    7,   // DISP_D12_PIN = GPIO7
                    15,  // DISP_D13_PIN = GPIO15
                    8,   // DISP_D14_PIN = GPIO8
                    16,  // DISP_D15_PIN = GPIO16
                },
            .bus_width = 16,  // DISP_BITS_WIDTH
            .max_transfer_bytes = DISP_BUF_SIZE * sizeof(uint16_t),
            .psram_trans_align = 64,
            .sram_trans_align = 4,
        },
    .io_config =
        {
            .cs_gpio_num = -1,
            .pclk_hz =
                (8 * 1000 *
                 1000),  // could be 10 if no PSRAM memory= DISP_CLK_FREQ,
            .trans_queue_depth = 10,
            .dc_levels =
                {
                    .dc_idle_level = 0,
                    .dc_cmd_level = 0,
                    .dc_dummy_level = 0,
                    .dc_data_level = 1,
                },
            .on_color_trans_done =
                NULL,  // Callback invoked when color data
                       // transfer has finished updated in bdsp.c
            .user_ctx =
                NULL,  // User private data, passed directly to
                       // on_color_trans_done’s user_ctx updated in bdsp.c
            .lcd_cmd_bits = 16,    // DISP_CMD_BITS_WIDTH
            .lcd_param_bits = 16,  // DISP_PARAM_BITS_WIDTH
        },
    .panel_config =
        {
            .reset_gpio_num = 21,  // DISP_RST_PIN = GPIO21
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
            .data_endian =
                0, /*!< Set the data endian for color data larger than 1 byte */
            .bits_per_pixel = 16, /*!< Color depth, in bpp */
            .flags =
                {
                    .reset_active_high = 0, /*!< Setting this if the panel reset
                                               is high level active */
                },
            .vendor_config = NULL, /*!< Vendor specific configuration */
        },
    .orientation = DISP_ORIENTATION,
    .hor_res = DISP_HOR_RES_MAX,
    .ver_res = DISP_VER_RES_MAX,
};
```

* st7796 i80 driver
The `st7796` driver is a display driver that is used to control the ST7796 display. The `st7796` driver configuration is part of display driver configuration.

disp_def.h:
```cpp

#define DISP_ORIENTATION \
  (orientation_landscape)  // 0:portrait mode 1:landscape mode 2:portrait mode
                           // reverse 3:landscape mode reverse

#if DISP_ORIENTATION == orientation_landscape || \
    DISP_ORIENTATION == orientation_landscape_invert  // landscape mode
#define DISP_HOR_RES_MAX (480)
#define DISP_VER_RES_MAX (320)
#else  // portrait mode
#define DISP_HOR_RES_MAX (320)
#define DISP_VER_RES_MAX (480)
#endif

#define DISP_BUF_SIZE (DISP_HOR_RES_MAX * (DISP_VER_RES_MAX / 10))

const esp_i80_st7796_config_t st7796_cfg = {
    .bus_config =
        {
            .clk_src = LCD_CLK_SRC_DEFAULT,
            .dc_gpio_num = 0,  // DISP_RS_PIN=0
            .wr_gpio_num = 47,  // DISP_WR_PIN=GPIO47
            .data_gpio_nums =
                {
                    9,   // DISP_D00_PIN = GPIO9
                    46,   // DISP_D01_PIN = GPIO46
                    3,   // DISP_D02_PIN = GPIO3
                    8,  // DISP_D03_PIN = GPIO8
                    18,   // DISP_D04_PIN = GPIO18
                    17,  // DISP_D05_PIN = GPIO17
                    16,   // DISP_D06_PIN = GPIO16
                    15,  // DISP_D07_PIN = GPIO15
                    -1,   // DISP_D08_PIN = N/C
                    -1,  // DISP_D09_PIN = N/C
                    -1,   // DISP_D10_PIN = N/C
                    -1,  // DISP_D11_PIN = N/C
                    -1,   // DISP_D12_PIN = N/C
                    -1,  // DISP_D13_PIN = N/C
                    -1,   // DISP_D14_PIN = N/C
                    -1,  // DISP_D15_PIN = N/C
                },
            .bus_width = 8,  // DISP_BITS_WIDTH
            .max_transfer_bytes = DISP_BUF_SIZE * sizeof(uint16_t),
            .psram_trans_align = 64,
            .sram_trans_align = 4,
        },
    .io_config =
        {
            .cs_gpio_num = 48, //DISP_CS_PIN
            .pclk_hz =
                (8 * 1000 *
                 1000),  // could be 10 if no PSRAM memory= DISP_CLK_FREQ,
            .trans_queue_depth = 10,
            .dc_levels =
                {
                    .dc_idle_level = 0,
                    .dc_cmd_level = 0,
                    .dc_dummy_level = 0,
                    .dc_data_level = 1,
                },
            .on_color_trans_done =
                NULL,  // Callback invoked when color data
                       // transfer has finished updated in bdsp.c
            .user_ctx =
                NULL,  // User private data, passed directly to
                       // on_color_trans_done’s user_ctx updated in bdsp.c
            .lcd_cmd_bits = 8,    // DISP_CMD_BITS_WIDTH
            .lcd_param_bits = 8,  // DISP_PARAM_BITS_WIDTH
        },
    .panel_config =
        {
            .reset_gpio_num = 4,  // DISP_RST_PIN = GPIO4 - Same as touch
            .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
            .data_endian =
                0, /*!< Set the data endian for color data larger than 1 byte */
            .bits_per_pixel = 16, /*!< Color depth, in bpp */
            .flags =
                {
                    .reset_active_high = 0, /*!< Setting this if the panel reset
                                               is high level active */
                },
            .vendor_config = NULL, /*!< Vendor specific configuration */
        },
    .orientation = DISP_ORIENTATION,
    .hor_res = DISP_HOR_RES_MAX,
    .ver_res = DISP_VER_RES_MAX,
};

### Backlight drivers
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

### SPI Touch drivers
* xpt2046 driver
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

### I2C Touch drivers

* gt911 driver
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
    .x_max = 0,//auto detect
    .y_max = 0,//auto detect    
};
```

* ft5x06 driver
The `ft5x06` driver is a touch driver that is used to control the FT5x06 touch controller. The `ft5x06` driver configuration is part of the touch driver configuration file : touch_def.h.

touch_def.h:
```cpp
const ft5x06_config_t ft5x06_cfg = {
    .i2c_clk_speed = 400*1000,
    .i2c_addr =
        (uint8_t[]){
            0x38,
            0}, 
    .rst_pin = -1, // GPIO 38
#if WITH_FT5X06_INT
    .int_pin = 18, // GPIO 18
#else
    .int_pin = -1, // INT pin not connected (by default)
#endif
    .swap_xy = true,
    .invert_x = false,
    .invert_y = false,
    .x_max = 480,
    .y_max = 800,     
};    
```
