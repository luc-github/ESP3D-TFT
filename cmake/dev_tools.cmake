# ESP3D-TFT specific log level
# All  = 3
# Debug = 2
# Error only = 1
# Disabled = 0
add_compile_options(-DESP3D_TFT_LOG=0)
# Disable ANSI color to fit some serial terminals 
add_compile_options(-DDISABLE_COLOR_LOG=0)

#Use the Snapshot API of LVGL to dump screens to the SD card
add_compile_options(-DLV_USE_SNAPSHOT=0)

# ESP3D-TFT specific bechmark
# Enabled  = 1
# Disabled = 0
add_compile_options(-DESP3D_TFT_BENCHMARK=0)