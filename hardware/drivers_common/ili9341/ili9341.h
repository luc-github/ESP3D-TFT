/**
 * @file ili9341.h
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
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

/**********************
 *      TYPEDEFS
 **********************/

/**
 * @brief Enumeration for the orientation of the display.
 *
 * This enumeration defines the possible orientations of the display.
 * - `orientation_portrait`: The display is in portrait mode.
 * - `orientation_landscape`: The display is in landscape mode.
 * - `orientation_portrait_invert`: The display is in inverted portrait mode.
 * - `orientation_landscape_invert`: The display is in inverted landscape mode.
 */
typedef enum {
  orientation_portrait = 0,
  orientation_landscape = 1,
  orientation_portrait_invert = 2,
  orientation_landscape_invert = 3,
} esp_spi_ili9341_orientation_t;

typedef struct {
  uint32_t spi_host_index;
  int16_t pin_miso;        /**< MISO pin number */
  int16_t pin_mosi;        /**< MOSI pin number */
  int16_t pin_clk;         /**< CLK pin number */
  bool is_master;          /**< SPI master mode */
  int16_t max_transfer_sz; /**< Maximum transfer size */
  int16_t dma_channel;     /**< DMA channel */
  int16_t quadwp_io_num;   /**< QuadWP pin number */
  int16_t quadhd_io_num;   /**< QuadHD pin number */
} esp_spi_bus_ili9341_config_t;

/**
 * @brief Configuration structure for the ILI9341 driver.
 */

typedef struct {
  esp_lcd_panel_dev_config_t
      panel_dev_config;                      /**< Panel device configuration */
  esp_spi_bus_ili9341_config_t spi_bus_config;   /**< SPI configuration */
  esp_lcd_panel_io_spi_config_t disp_spi_cfg;
  esp_spi_ili9341_orientation_t orientation; /**< Orientation of the display */
  uint16_t hor_res; /**< Horizontal resolution of the display */
  uint16_t ver_res; /**< Vertical resolution of the display */
} esp_spi_ili9341_config_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
esp_err_t esp_lcd_new_panel_ili9341(const esp_lcd_panel_io_handle_t io, const esp_spi_ili9341_config_t *panel_cfg, esp_lcd_panel_handle_t *ret_panel);


#ifdef __cplusplus
} /* extern "C" */
#endif
