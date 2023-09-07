# Component directive
if(MDNS_SERVICE)
    set(mdnslib mdns)
endif()

#if(TFT_UI_SERVICE)
#    set(lvgllib lvgl)
#endif()

idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES ${mdnslib} esp3d_log lvgl spi_bus disp_spi ili9341 sw_spi xpt2046 disp_backlight
)