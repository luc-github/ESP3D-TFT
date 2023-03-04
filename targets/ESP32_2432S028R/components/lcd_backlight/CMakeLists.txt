if(TFT_UI_SERVICE)
    # Component directive
    set(SOURCES "esp_lcd_backlight.c")
    set(INCLUDES .)
    idf_component_register(SRCS ${SOURCES}
        INCLUDE_DIRS ${INCLUDES}
        REQUIRES esp3d_log driver)
endif()

unset(TFT_UI_SERVICE CACHE)