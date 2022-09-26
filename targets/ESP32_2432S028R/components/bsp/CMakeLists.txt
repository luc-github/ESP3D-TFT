# Component directive
idf_component_register(SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES lvgl xpt2046 ili9341 spi_bus disp_spi touch_spi lcd_backlight
)