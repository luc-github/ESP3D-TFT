# Component directive
if(MDNS_SERVICE)
    set(mdnslib mdns)
endif()

if(TFT_UI_SERVICE)
    set(lvgllib lvgl)
endif()

idf_component_register(SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES ${mdnslib} esp3d_log ${lvgllib} i2c_bus gt911 st7796 spi_bus lcd_backlight
)