/**
 * @file sw_spi.h
 *
 */

#ifndef SW_SPI_H
#define SW_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
  int8_t cs_pin;
  int8_t clk_pin;
  int8_t mosi_pin;
  int8_t miso_pin;
} sw_spi_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void sw_spi_init(const sw_spi_config_t *config);
uint16_t sw_spi_read_reg16(uint8_t reg);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*SW_SPI_H*/
