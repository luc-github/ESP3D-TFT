
#Check Target Firmware

if (NOT(TARGET_FW_MARLIN || TARGET_FW_REPETIER || TARGET_FW_SMOOTHIEWARE || TARGET_FW_GRBL))
    message(FATAL_ERROR
        "\n"
        "No firmware target defined, please define a target in CMakeLists.txt"
        "\n"
        "Now cmake will exit")
endif()

#Check Target Board
if (NOT (TFT_TARGET))
    message(FATAL_ERROR
        "\n"
        "No hardware target defined, please define a target in CMakeLists.txt"
        "\n"
        "Now cmake will exit")
endif()