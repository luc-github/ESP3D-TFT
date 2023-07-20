# Component directive
idf_component_register(SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl i2c_bus gt911 ek9716 lcd_backlight
)