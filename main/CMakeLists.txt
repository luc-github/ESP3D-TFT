# main application directives

set(SOURCES .
    core core/commands
    modules/authentication
    modules/serial
    modules/network
    modules/filesystem
    modules/filesystem/flash
    modules/usb_serial
)

if(WIFI_SERVICE)
    if(SSDP_SERVICE)
        list(APPEND SOURCES "modules/ssdp")
    endif()

    if(MDNS_SERVICE)
        list(APPEND SOURCES "modules/mdns")
    endif()

    if(WEB_SERVICES)
        list(APPEND SOURCES "modules/http")
        list(APPEND SOURCES "modules/http/handlers")
        list(APPEND SOURCES "modules/websocket")
        set(WEBEMBEDDEDFILES "embedded/favicon.ico.gz" "embedded/index.html.gz")
    endif()

    if(TELNET_SERVICE)
        list(APPEND SOURCES "modules/socket_server")
    endif()

    if(NOTIFICATIONS_SERVICE)
        list(APPEND SOURCES "modules/notifications")
    endif()
endif()

if(BT_SERVICE)
    # TODO
endif()

if(SD_CARD_SERVICE)
    list(APPEND SOURCES "modules/filesystem/sd")
endif()

if(UPDATE_SERVICE)
    list(APPEND SOURCES "modules/update")
endif()

if(GCODE_HOST_SERVICE)
    list(APPEND SOURCES "modules/gcode_host")
endif()

 if(TFT_UI_SERVICE)
    list(APPEND SOURCES "display")
endif()

set(INCLUDES .
    core/includes
    modules)
idf_component_register(SRC_DIRS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES}
    EMBED_FILES ${WEBEMBEDDEDFILES}  ${UIEMBEDDEDFILES})

unset(TFT_UI_SERVICE CACHE)
unset(WIFI_SERVICE CACHE)
unset(SSDP_SERVICE CACHE)
unset(MDNS_SERVICE CACHE)
unset(BT_SERVICE CACHE)
unset(WEB_SERVICES CACHE)
unset(TELNET_SERVICE CACHE)
unset(WS_SERVICE CACHE)
unset(SD_CARD_SERVICE CACHE)
unset(NOTIFICATIONS_SERVICE CACHE)
unset(UPDATE_SERVICE CACHE)
unset(GCODE_HOST_SERVICE CACHE)
