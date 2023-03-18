# Component directive
set(SOURCES "ili9488.c")
set(INCLUDES .)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES esp3d_log lvgl disp_spi
)
