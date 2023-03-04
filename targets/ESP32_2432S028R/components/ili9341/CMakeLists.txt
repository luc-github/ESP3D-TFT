# Component directive
if(TFT_UI_SERVICE)
    set(SOURCES "ili9341.c")
    set(INCLUDES .)
    idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS ${INCLUDES}
        REQUIRES esp3d_log lvgl disp_spi
    )
endif()

unset(TFT_UI_SERVICE CACHE)