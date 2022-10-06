# Component directive
idf_component_register(SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl xpt2046 ili9341 spi_bus disp_spi touch_spi lcd_backlight
)