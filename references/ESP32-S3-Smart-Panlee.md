# ESP32-S3-Smart Panlee
## Github   
?

## Specs
* ESP32-S3 dual-core Xtensa 32-bit LX7 microprocessor, up to 240 MHz with 384KB ROM, 512KB SRAM. 2.4GHz WiFi and Bluetooth 5
* PSRAM: 2MB     
* FLASH: 8MB
* Micro-SD card slot (SPI)
* 3.5-inch display with 480×320 ST7796UI (8080 parallel bus)  RGB565
* I2C capacitive touch panel ft5x06 (i2C 0x38)
* Audio (NS4168)
* 1 USB-C OTG (DFU/CDC) port
* Wakeup and reset buttons, 
* Power switch
* Power Supply: 5V / 1A
* Dimension: 110 x 61 x 13.5mm   
* 1 Debug interface (7pins): V5 - V3.3 - TX - RX - EN - GPIO 0 - GND / GPIO 14 
* 1 Extended IO interface (8pins): V5 - GND - EXT_IO1- EXT_IO2- EXT_IO3- EXT_IO4- EXT_IO5- EXT_IO6
* 1 RS485 interface (4pins): RS485-A - RS485-B - GND - V5

## Pinout 
Pin | Usage 
----|-----
GPIO 0 | LCD_RS  
GPIO 1 |  RS485_RX
GPIO 2 |  RS485_RTS 
GPIO 3 | LCD_D2
GPIO 4 | LCD_RESET / TOUCH_RESET
GPIO 5 | TOUCH_SCL
GPIO 6 | TOUCH_SDA
GPIO 7 | TOUCH_INT 
GPIO 8 | LCD_D3 
GPIO 9 | LCD_D0
GPIO 10 | EXT_IO1
GPIO 11 | EXT_IO2
GPIO 12 | EXT_IO3
GPIO 13 | EXT_IO4
GPIO 14 | EXT_IO5
GPIO 15 | LCD_D7
GPIO 16 | LCD_D6
GPIO 17 | LCD_D5
GPIO 18 | LCD_D4
GPIO 19 | 
GPIO 20 | 
GPIO 21 | EXT_IO6
GPIO 33 |  
GPIO 34 | 
GPIO 35 | AUDIO_LRCK
GPIO 36 | AUDIO_BCLK
GPIO 37 | AUDIO_DOUT
GPIO 38 | SD_MISO 
GPIO 39 | SD_CLK
GPIO 40 | SD_MOSI
GPIO 41 | SD_CS
GPIO 42 | RS485_TX
GPIO 43 | U0TXD
GPIO 44 | U0RXD
GPIO 45 | LCD_BL (High)
GPIO 46 | LCD_D1  
GPIO 47 | LCD_WR
GPIO 48 | LCD_TE
