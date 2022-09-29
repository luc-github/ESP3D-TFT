# Component directive
set(SOURCES "st7796.c")
set(INCLUDES .)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES lvgl esp_lcd
)
