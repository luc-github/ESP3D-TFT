# Component directive
set(SOURCES "ili9341.c")
set(INCLUDES .)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES lvgl disp_spi
)
