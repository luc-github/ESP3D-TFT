# Component directive
idf_component_register(SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES mdns esp3d_log lvgl xpt2046_no_spi ili9341 spi_bus disp_spi lcd_backlight
)