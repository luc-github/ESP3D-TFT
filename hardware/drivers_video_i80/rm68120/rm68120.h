/**
 * @file rm68120.h
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_commands.h"

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    orientation_portrait = 0,
    orientation_landscape = 1,
    orientation_portrait_invert = 2,
    orientation_landscape_invert = 3,
} esp_i80_rm68120_orientation_t;

typedef struct {
    esp_lcd_i80_bus_config_t bus_config ;
    esp_lcd_panel_io_i80_config_t io_config;
    esp_lcd_panel_dev_config_t panel_config;
    esp_i80_rm68120_orientation_t orientation;
    uint16_t hor_res;
    uint16_t ver_res;
} esp_i80_rm68120_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * @brief Initializes the RM68120 display driver.
 *
 * This function initializes the RM68120 display driver with the provided configuration.
 *
 * @param disp_rm68120_cfg Pointer to the configuration structure for the RM68120 display driver.
 * @param panel_handle Pointer to the handle of the LCD panel.
 * @param flush_ready_fn Pointer to the flush ready function.
 * @return `ESP_OK` if the initialization is successful, otherwise an error code.
 */
esp_err_t rm68120_init(const esp_i80_rm68120_config_t *disp_rm68120_cfg,esp_lcd_panel_handle_t *panel_handle,  void * flush_ready_fn);

#ifdef __cplusplus
} /* extern "C" */
#endif
