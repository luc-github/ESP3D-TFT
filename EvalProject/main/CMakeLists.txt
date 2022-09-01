
OPTION(USE_OPTION1 "OPTION 1" OFF)
OPTION(USE_OPTION2 "OPTION 2" ON)
set(SRCS main.c)

set(OPTION1_SRC modules/option1/option1.c)
set(OPTION2_SRC modules/option2/option2.c)

if(USE_OPTION1)
    # target_compile_definitions("${COMPONENT_LIB}" PUBLIC USE_OPTION1=1)
    list(APPEND SRCS ${OPTION1_SRC})
endif()

if(USE_OPTION2)
    # target_compile_definitions("${COMPONENT_LIB}" PUBLIC USE_OPTION_2=1)
    list(APPEND SRCS ${OPTION2_SRC})
endif()

idf_component_register(SRCS "${SRCS}" INCLUDE_DIRS ".")

if(USE_OPTION1)
    target_compile_definitions("${COMPONENT_LIB}" PUBLIC USE_OPTION1=1)
endif()

if(USE_OPTION2)
    target_compile_definitions("${COMPONENT_LIB}" PUBLIC USE_OPTION_2=1)
endif()