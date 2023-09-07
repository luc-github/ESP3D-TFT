/**
 * @file lv_templ.h
 *
 */

#ifndef ILI9341_H
#define ILI9341_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
  int8_t rst_pin;
  int8_t dc_pin;
} ili9341_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

esp_err_t ili9341_init(const ili9341_config_t *config);
void ili9341_set_invert_color(bool invert);
void ili9341_set_orientation(uint8_t orientation);
void ili9341_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data);
void ili9341_sleep_in(void);
void ili9341_sleep_out(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*ILI9341_H*/
