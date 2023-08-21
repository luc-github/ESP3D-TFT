# Component directive
set(SOURCES "st7262.c")
set(INCLUDES .)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES esp3d_log lvgl esp_lcd driver
)
