/**
 * @file XPT2046.h
 *
 */

#ifndef XPT2046_H
#define XPT2046_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef uint16_t (*xpt2046_read_reg12_fn_t)(uint8_t reg);

typedef struct {
  int8_t irq_pin;
  xpt2046_read_reg12_fn_t read_reg12_fn;
  uint16_t touch_threshold;
  bool swap_xy;
  bool invert_x;
  bool invert_y;
} xpt2046_config_t;

typedef struct {
  int16_t x;
  int16_t y;
  bool is_pressed;
} xpt2046_data_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
esp_err_t xpt2046_init(const xpt2046_config_t *config);
xpt2046_data_t xpt2046_read();

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XPT2046_H */
