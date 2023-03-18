# Component directive
if(MDNS_SERVICE)
    set(mdnslib mdns)
endif()

if(TFT_UI_SERVICE)
    set(lvgllib lvgl)
endif()

idf_component_register(SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES ${mdnslib} esp3d_log ${lvgllib} xpt2046_no_spi ili9341 spi_bus disp_spi lcd_backlight
)