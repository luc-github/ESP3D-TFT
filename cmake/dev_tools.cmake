# ESP3D-TFT Development Configuration

# ===========================================
# Logging Configuration
# ===========================================

# ESP3D-TFT Log Level
# 3 = All
# 2 = Debug
# 1 = Error only
# 0 = Disabled
set(ESP3D_TFT_LOG_LEVEL 2)
add_compile_options(-DESP3D_TFT_LOG=${ESP3D_TFT_LOG_LEVEL})

# ANSI Color in Logs
# 0 = Enabled (default)
# 1 = Disabled (for compatibility with some serial terminals)
set(DISABLE_COLOR_LOG 0)
add_compile_options(-DDISABLE_COLOR_LOG=${DISABLE_COLOR_LOG})

# ===========================================
# LVGL Configuration
# ===========================================

# Use the Snapshot API of LVGL to dump screens to the SD card
set(LV_USE_SNAPSHOT 0)
add_compile_options(-DLV_USE_SNAPSHOT=${LV_USE_SNAPSHOT})

# ===========================================
# Telnet Configuration
# ===========================================
# Disable telnet server welcome message for compatibility with some serial terminals
# 1 = Disable telnet server welcome message
# 0 = Keep telnet server welcome message

set(DISABLE_TELNET_WELCOME_MESSAGE 0)
add_compile_options(-DDISABLE_TELNET_WELCOME_MESSAGE=${DISABLE_TELNET_WELCOME_MESSAGE})

# ===========================================
# Performance Configuration
# ===========================================

# ESP3D-TFT Benchmark
# 1 = Enabled
# 0 = Disabled
set(ESP3D_TFT_BENCHMARK 0)
add_compile_options(-DESP3D_TFT_BENCHMARK=${ESP3D_TFT_BENCHMARK})
