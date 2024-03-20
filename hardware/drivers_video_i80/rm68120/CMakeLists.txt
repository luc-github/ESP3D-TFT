# Component directive
set(SOURCES "rm68120.c")
set(INCLUDES .)
idf_component_register(SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    REQUIRES esp3d_log esp_lcd driver
)
