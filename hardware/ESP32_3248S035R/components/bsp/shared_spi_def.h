// Shared SPI definitions for ESP32_3248S035R
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "spi_bus.h"

// Shared SPI (Display, Touch)
#define SHARED_SPI_HOST SPI2_HOST  // 1
// #define SHARED_SPI_HOST SPI3_HOST // 2

#define SHARED_SPI_CLK  14  // GPIO 14
#define SHARED_SPI_MOSI 13  // GPIO 13
#define SHARED_SPI_MISO 12  // GPIO 12

#ifdef __cplusplus
} /* extern "C" */
#endif
