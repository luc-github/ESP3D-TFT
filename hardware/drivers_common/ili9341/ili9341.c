/*
  ili9341.c

  Copyright (c) 2023 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "ili9341.h"

#include <driver/gpio.h>
#include <stdlib.h>
#include <sys/cdefs.h>

#include "esp3d_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
  esp_lcd_panel_t base;
  esp_lcd_panel_io_handle_t io;
  int reset_gpio_num;
  bool reset_level;
  int x_gap;
  int y_gap;
  unsigned int bits_per_pixel;
  uint8_t madctl_val;  // save current value of LCD_CMD_MADCTL register
  uint8_t colmod_cal;  // save current value of LCD_CMD_COLMOD register
} lcd_panel_t;

/* The LCD needs a bunch of command/argument values to be initialized. They are
 * stored in this struct. */
typedef struct {
  uint8_t cmd;
  uint8_t data[16];
  uint8_t num_bytes;  // Number of data bytes; 0xFF = end of cmds.
} lcd_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static esp_err_t lcd_panel_del(esp_lcd_panel_t *panel);
static esp_err_t lcd_panel_reset(esp_lcd_panel_t *panel);
static esp_err_t lcd_panel_init(esp_lcd_panel_t *panel);
static esp_err_t lcd_panel_draw_bitmap(esp_lcd_panel_t *panel, int x_start,
                                       int y_start, int x_end, int y_end,
                                       const void *color_data);
static esp_err_t lcd_panel_invert_color(esp_lcd_panel_t *panel,
                                        bool invert_color_data);
static esp_err_t lcd_panel_mirror(esp_lcd_panel_t *panel, bool mirror_x,
                                  bool mirror_y);
static esp_err_t lcd_panel_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t lcd_panel_set_gap(esp_lcd_panel_t *panel, int x_gap,
                                   int y_gap);
static esp_err_t lcd_panel_disp_on_off(esp_lcd_panel_t *panel, bool off);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes a new ILI9341 LCD panel.
 *
 * This function initializes a new ILI9341 LCD panel using the provided I/O
 * handle and panel configuration.
 *
 * @param io The I/O handle for the LCD panel.
 * @param panel_cfg The configuration settings for the ILI9341 panel.
 * @param disp_panel Pointer to the variable where the LCD panel handle will be
 * stored.
 *
 * @return `ESP_OK` if the panel is successfully initialized, or an error code
 * if initialization fails.
 */
esp_err_t esp_lcd_new_panel_ili9341(const esp_lcd_panel_io_handle_t io,
                                    const esp_spi_ili9341_config_t *panel_cfg,
                                    esp_lcd_panel_handle_t *disp_panel) {
  esp_err_t ret = ESP_OK;
  lcd_panel_t *lcd_panel = NULL;
  ESP_GOTO_ON_FALSE(io && panel_cfg && disp_panel, ESP_ERR_INVALID_ARG, err, "",
                    "invalid argument");
  lcd_panel = calloc(1, sizeof(lcd_panel_t));
  ESP_GOTO_ON_FALSE(lcd_panel, ESP_ERR_NO_MEM, err, "",
                    "no mem for ili9341 panel");

  if (panel_cfg->panel_dev_config.reset_gpio_num >= 0) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << panel_cfg->panel_dev_config.reset_gpio_num,
    };
    ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, "",
                      "configure GPIO for RST line failed");
  }

  switch (panel_cfg->panel_dev_config.rgb_ele_order) {
    case LCD_RGB_ELEMENT_ORDER_RGB:
      lcd_panel->madctl_val = 0;
      break;
    case LCD_RGB_ELEMENT_ORDER_BGR:
      lcd_panel->madctl_val |= LCD_CMD_BGR_BIT;
      break;
    default:
      ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "",
                        "unsupported color space");
      break;
  }

  switch (panel_cfg->panel_dev_config.bits_per_pixel) {
    case 16:
      lcd_panel->colmod_cal = 0x55;
      break;
    case 18:
      lcd_panel->colmod_cal = 0x66;
      break;
    default:
      ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "",
                        "unsupported pixel width");
      break;
  }

  lcd_panel->io = io;
  lcd_panel->bits_per_pixel = panel_cfg->panel_dev_config.bits_per_pixel;
  lcd_panel->reset_gpio_num = panel_cfg->panel_dev_config.reset_gpio_num;
  lcd_panel->reset_level = panel_cfg->panel_dev_config.flags.reset_active_high;
  lcd_panel->base.del = lcd_panel_del;
  lcd_panel->base.reset = lcd_panel_reset;
  lcd_panel->base.init = lcd_panel_init;
  lcd_panel->base.draw_bitmap = lcd_panel_draw_bitmap;
  lcd_panel->base.invert_color = lcd_panel_invert_color;
  lcd_panel->base.set_gap = lcd_panel_set_gap;
  lcd_panel->base.mirror = lcd_panel_mirror;
  lcd_panel->base.swap_xy = lcd_panel_swap_xy;
  lcd_panel->base.disp_on_off = lcd_panel_disp_on_off;
  *disp_panel = &(lcd_panel->base);
  esp3d_log("new ili9341 panel @%p", lcd_panel);
  // reset ili9341 panel
  ESP_GOTO_ON_ERROR(esp_lcd_panel_reset(*disp_panel), err, "",
                    "reset ili9341 panel failed");
  // init ili9341 panel
  ESP_GOTO_ON_ERROR(esp_lcd_panel_init(*disp_panel), err, "",
                    "init ili9341 panel failed");
  // set Orientation
  if (panel_cfg->orientation == orientation_landscape ||
      panel_cfg->orientation == orientation_landscape_invert) {
    ESP_GOTO_ON_ERROR(esp_lcd_panel_swap_xy(*disp_panel, true), err, "",
                      "swap ili9341 panel failed");
  }
  if (panel_cfg->orientation == orientation_portrait_invert ||
      panel_cfg->orientation == orientation_landscape_invert) {
    ESP_GOTO_ON_ERROR(esp_lcd_panel_mirror(*disp_panel, true, true), err, "",
                      "mirror ili9341 panel failed");
  }

  return ESP_OK;

err:
  if (lcd_panel) {
    if (panel_cfg->panel_dev_config.reset_gpio_num >= 0) {
      gpio_reset_pin(panel_cfg->panel_dev_config.reset_gpio_num);
    }
    free(lcd_panel);
  }
  return ret;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief Deletes an LCD panel.
 *
 * This function is used to delete an LCD panel structure and free up the memory
 * allocated for it.
 *
 * @param panel Pointer to the LCD panel structure to be deleted.
 * @return `ESP_OK` if the LCD panel is successfully deleted, or an error code
 * if an error occurred.
 */
static esp_err_t lcd_panel_del(esp_lcd_panel_t *panel) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);

  if (lcd_panel->reset_gpio_num >= 0) {
    gpio_reset_pin(lcd_panel->reset_gpio_num);
  }
  esp3d_log("del ili9341 panel @%p", lcd_panel);
  free(lcd_panel);
  return ESP_OK;
}

/**
 * @brief Resets the LCD panel.
 *
 * This function is responsible for resetting the LCD panel specified by the
 * `panel` parameter.
 *
 * @param panel Pointer to the `esp_lcd_panel_t` structure representing the LCD
 * panel.
 * @return `ESP_OK` if the LCD panel reset is successful, otherwise an error
 * code indicating the cause of failure.
 */
static esp_err_t lcd_panel_reset(esp_lcd_panel_t *panel) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;

  // perform hardware reset
  if (lcd_panel->reset_gpio_num >= 0) {
    gpio_set_level(lcd_panel->reset_gpio_num, lcd_panel->reset_level);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(lcd_panel->reset_gpio_num, !lcd_panel->reset_level);
    vTaskDelay(pdMS_TO_TICKS(10));
  } else {  // perform software reset
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(
        20));  // spec, wait at least 5 ms. before sending new command
  }
  return ESP_OK;
}

/**
 * @brief Initializes the LCD panel.
 *
 * This function initializes the LCD panel specified by the `panel` parameter.
 *
 * @param panel Pointer to the `esp_lcd_panel_t` structure representing the LCD
 * panel.
 * @return `ESP_OK` if the initialization is successful, otherwise an error
 * code.
 */
static esp_err_t lcd_panel_init(esp_lcd_panel_t *panel) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;

  // LCD goes into sleep mode and display will be turned off after power on
  // reset, exit sleep mode first
  esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(100));

  lcd_cmd_t vendor_init_cmds[] = {
      {0xCF, {0x00, 0x83, 0X30}, 3},              // Power control B
      {0xED, {0x64, 0x03, 0X12, 0X81}, 4},        // Power on sequence control
      {0xE8, {0x85, 0x01, 0x79}, 3},              // Driver timing control A
      {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},  // Power control A
      {0xF7, {0x20}, 1},                          // Pump ratio control
      {0xEA, {0x00, 0x00}, 2},                    // Driver timing control B
      {0xC0, {0x26}, 1},                          // Power Control 1
      {0xC1, {0x11}, 1},                          // Power Control 2
      {0xC5, {0x35, 0x3E}, 2},                    // VCOM Control 1
      {0xC7, {0xBE}, 1},                          // VCOM Control 2
      {0xB1,
       {0x00, 0x1B},
       2},                // Frame Rate Control (In Normal Mode/Full Colors)
      {0xF2, {0x08}, 1},  // Enable 3G (3 gamma control)
      {0x26, {0x01}, 1},  // Gamma Set
      {0xE0,
       {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02,
        0x07, 0x05, 0x00},
       15},  // Positive Gamme Correction
      {0XE1,
       {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D,
        0x38, 0x3A, 0x1F},
       15},                                 // Negative Gamme Correction
      {0xB7, {0x07}, 1},                    // Entry Mode Set
      {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},  // Display Function Control
      {0, {0}, 0xFF},                       // NOP
  };

  // Send all the vendor init commands
  uint8_t i = 0;
  while (vendor_init_cmds[i].num_bytes != 0xFF) {
    esp_lcd_panel_io_tx_param(io, vendor_init_cmds[i].cmd,
                              vendor_init_cmds[i].data,
                              vendor_init_cmds[i].num_bytes);
    i++;
  }

  // Memory Data Access Control
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL,
                            (uint8_t[]){
                                lcd_panel->madctl_val,
                            },
                            1);
  // Interface Pixel Format
  esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD,
                            (uint8_t[]){
                                lcd_panel->colmod_cal,
                            },
                            1);

  // Turn on display
  esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);

  return ESP_OK;
}

/**
 * @brief  Draws a bitmap on the LCD panel.
 *
 * This function draws a bitmap on the LCD panel starting from the specified
 * coordinates (x_start, y_start) and ending at the specified coordinates
 * (x_end, y_end). The bitmap data is provided through the color_data parameter.
 *
 * @param panel       Pointer to the LCD panel structure.
 * @param x_start     The starting x-coordinate of the bitmap.
 * @param y_start     The starting y-coordinate of the bitmap.
 * @param x_end       The ending x-coordinate of the bitmap.
 * @param y_end       The ending y-coordinate of the bitmap.
 * @param color_data  Pointer to the color data of the bitmap.
 *
 * @return            ESP_OK if the bitmap was successfully drawn, otherwise an
 *                    appropriate error code is returned.
 */
static esp_err_t lcd_panel_draw_bitmap(esp_lcd_panel_t *panel, int x_start,
                                       int y_start, int x_end, int y_end,
                                       const void *color_data) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  assert((x_start < x_end) && (y_start < y_end) &&
         "start position must be smaller than end position");
  esp_lcd_panel_io_handle_t io = lcd_panel->io;

  x_start += lcd_panel->x_gap;
  x_end += lcd_panel->x_gap;
  y_start += lcd_panel->y_gap;
  y_end += lcd_panel->y_gap;

  // define an area of frame memory where MCU can access
  esp_lcd_panel_io_tx_param(io, LCD_CMD_CASET,
                            (uint8_t[]){
                                (x_start >> 8) & 0xFF,
                                x_start & 0xFF,
                                ((x_end - 1) >> 8) & 0xFF,
                                (x_end - 1) & 0xFF,
                            },
                            4);
  esp_lcd_panel_io_tx_param(io, LCD_CMD_RASET,
                            (uint8_t[]){
                                (y_start >> 8) & 0xFF,
                                y_start & 0xFF,
                                ((y_end - 1) >> 8) & 0xFF,
                                (y_end - 1) & 0xFF,
                            },
                            4);

  // transfer frame buffer
  size_t len =
      (x_end - x_start) * (y_end - y_start) * lcd_panel->bits_per_pixel / 8;
  esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, len);

  return ESP_OK;
}

/**
 * @brief Inverts the color data of the LCD panel.
 *
 * This function is used to invert the color data of the LCD panel. When
 * `invert_color_data` is set to `true`, the color data will be inverted. When
 * `invert_color_data` is set to `false`, the color data will remain unchanged.
 *
 * @param panel Pointer to the LCD panel structure.
 * @param invert_color_data Flag indicating whether to invert the color data.
 * @return `ESP_OK` if the color data is successfully inverted, or an error code
 * if an error occurred.
 */
static esp_err_t lcd_panel_invert_color(esp_lcd_panel_t *panel,
                                        bool invert_color_data) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;
  int command = 0;
  if (invert_color_data) {
    command = LCD_CMD_INVON;
  } else {
    command = LCD_CMD_INVOFF;
  }
  esp_lcd_panel_io_tx_param(io, command, NULL, 0);
  return ESP_OK;
}

/**
 * @brief Mirrors the LCD panel horizontally and/or vertically.
 *
 * This function allows you to mirror the LCD panel horizontally and/or
 * vertically.
 *
 * @param panel Pointer to the LCD panel structure.
 * @param mirror_x Set to `true` to mirror the panel horizontally, `false`
 * otherwise.
 * @param mirror_y Set to `true` to mirror the panel vertically, `false`
 * otherwise.
 * @return `ESP_OK` if the operation is successful, otherwise an error code.
 */
static esp_err_t lcd_panel_mirror(esp_lcd_panel_t *panel, bool mirror_x,
                                  bool mirror_y) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;
  if (mirror_x) {
    lcd_panel->madctl_val |= LCD_CMD_MX_BIT;
  } else {
    lcd_panel->madctl_val &= ~LCD_CMD_MX_BIT;
  }
  if (mirror_y) {
    lcd_panel->madctl_val |= LCD_CMD_MY_BIT;
  } else {
    lcd_panel->madctl_val &= ~LCD_CMD_MY_BIT;
  }
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL,
                            (uint8_t[]){lcd_panel->madctl_val}, 1);
  return ESP_OK;
}

/**
 * @brief Swaps the X and Y axes of the LCD panel.
 *
 * This function is used to swap the X and Y axes of the LCD panel if needed.
 *
 * @param panel Pointer to the LCD panel structure.
 * @param swap_axes Boolean value indicating whether to swap the axes.
 *                 - `true` to swap the axes.
 *                 - `false` to keep the axes as is.
 *
 * @return `ESP_OK` if the axes are successfully swapped, or an error code if an
 * error occurred.
 */
static esp_err_t lcd_panel_swap_xy(esp_lcd_panel_t *panel, bool swap_axes) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;
  if (swap_axes) {
    lcd_panel->madctl_val |= LCD_CMD_MV_BIT;
  } else {
    lcd_panel->madctl_val &= ~LCD_CMD_MV_BIT;
  }
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL,
                            (uint8_t[]){lcd_panel->madctl_val}, 1);
  return ESP_OK;
}

/**
 * Sets the gap between pixels on the LCD panel.
 *
 * @param panel The LCD panel structure.
 * @param x_gap The horizontal gap between pixels.
 * @param y_gap The vertical gap between pixels.
 * @return `ESP_OK` if the gap was set successfully, otherwise an error code.
 */
static esp_err_t lcd_panel_set_gap(esp_lcd_panel_t *panel, int x_gap,
                                   int y_gap) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  lcd_panel->x_gap = x_gap;
  lcd_panel->y_gap = y_gap;
  return ESP_OK;
}

/**
 * @brief Turns the display on or off for the LCD panel.
 *
 * This function is used to turn the display on or off for the specified LCD
 * panel.
 *
 * @param panel Pointer to the ESP_LCD_PANEL structure representing the LCD
 * panel.
 * @param off Boolean value indicating whether to turn the display off (true) or
 * on (false).
 *
 * @return ESP_OK if the operation is successful, otherwise an error code.
 */
static esp_err_t lcd_panel_disp_on_off(esp_lcd_panel_t *panel, bool off) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;
  int command = 0;
  if (off) {
    command = LCD_CMD_DISPOFF;
  } else {
    command = LCD_CMD_DISPON;
  }
  esp_lcd_panel_io_tx_param(io, command, NULL, 0);
  return ESP_OK;
}
