# main application directives
set(SOURCES . core)
set(INCLUDES . core/includes)
idf_component_register(SRC_DIRS ${SOURCES}
    INCLUDE_DIRS ${INCLUDES})
