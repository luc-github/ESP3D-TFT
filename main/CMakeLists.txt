# main application directives
set(SOURCES . core modules/network)
set(INCLUDES . core/includes modules/network)
idf_component_register(SRC_DIRS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES})
