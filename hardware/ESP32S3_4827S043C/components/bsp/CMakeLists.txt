# Component directive
idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS .
    REQUIRES esp3d_log lvgl esp_lcd i2c_bus ili9485 gt911 disp_backlight
)