/**
 * @file spi_bus.h
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>
#include <driver/spi_master.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void spi_bus_init(int host,
                  int miso_pin, int mosi_pin, int sclk_pin,
                  int max_transfer_sz,
                  int dma_channel,
                  int quadwp_pin, int quadhd_pin);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif
