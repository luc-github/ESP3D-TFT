if(TFT_UI_SERVICE)
    # Component directive
    set(SOURCES "xpt2046.c")
    set(INCLUDES .)
    idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS ${INCLUDES}
        REQUIRES esp3d_log lvgl driver
    )
endif()

unset(TFT_UI_SERVICE, CACHE)