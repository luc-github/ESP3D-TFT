# Component directive
idf_component_register(SRCS "disp_driver.c" "disp_driver.c" "disp_spi.c" "esp_lcd_backlight.c" "ili9488.c"
    INCLUDE_DIRS .
    REQUIRES lvgl lvgl_conf touch
)

target_compile_definitions("${COMPONENT_LIB}" PUBLIC LV_LVGL_H_INCLUDE_SIMPLE)
