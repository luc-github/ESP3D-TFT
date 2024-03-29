/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 * Modified by Luc LEBOSSE 2024
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "st7796.h"

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
/**
 * @brief Structure representing an LCD panel.
 *
 * This structure contains information about an LCD panel, including its base
 * properties, I/O handle, reset GPIO number, reset level, pixel gaps, bits per
 * pixel, and saved values of the MADCTL and COLMOD registers.
 */
typedef struct {
  esp_lcd_panel_t base;         /**< Base properties of the LCD panel */
  esp_lcd_panel_io_handle_t io; /**< I/O handle for the LCD panel */
  int reset_gpio_num;           /**< GPIO number for the reset pin */
  bool reset_level;             /**< Reset level for the reset pin */
  int x_gap;                    /**< Pixel gap in the X direction */
  int y_gap;                    /**< Pixel gap in the Y direction */
  unsigned int bits_per_pixel;  /**< Number of bits per pixel */
  uint8_t madctl_val; /**< Saved value of the LCD_CMD_MADCTL register */
  uint8_t colmod_cal; /**< Saved value of the LCD_CMD_COLMOD register */
} lcd_panel_t;

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
 * @brief Creates a new ST7796 SPI LCD panel.
 *
 * This function initializes a new ST7796 LCD panel using the provided I/O
 * handle and panel device configuration.
 *
 * @param io The I/O handle for the LCD panel.
 * @param panel_cfg The device configuration for the LCD panel.
 * @param disp_panel Pointer to store the created LCD panel handle.
 * @return `ESP_OK` if the LCD panel is successfully created, or an error code
 * if an error occurred.
 */
esp_err_t esp_lcd_new_panel_st7796(const esp_lcd_panel_io_handle_t io,
                                   const esp_spi_st7262_config_t *panel_cfg,
                                   esp_lcd_panel_handle_t *disp_panel) {
  esp_err_t ret = ESP_OK;
  lcd_panel_t *lcd_panel = NULL;
  ESP_GOTO_ON_FALSE(io && panel_cfg && disp_panel, ESP_ERR_INVALID_ARG, err, "",
                    "invalid argument");
  lcd_panel = calloc(1, sizeof(lcd_panel_t));
  ESP_GOTO_ON_FALSE(lcd_panel, ESP_ERR_NO_MEM, err, "",
                    "no mem for st7796 panel");

  if (panel_cfg->panel_dev_config.reset_gpio_num >= 0) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << panel_cfg->panel_dev_config.reset_gpio_num,
    };
    ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, "",
                      "configure GPIO for RST line failed");
  }

  switch (panel_cfg->panel_dev_config.color_space) {
    case ESP_LCD_COLOR_SPACE_RGB:
      lcd_panel->madctl_val = 0;
      break;
    case ESP_LCD_COLOR_SPACE_BGR:
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
  esp3d_log("new st7796 panel @%p", lcd_panel);

  // reset st7262 panel
  ESP_GOTO_ON_ERROR(esp_lcd_panel_reset(*disp_panel), err, "",
                    "reset st7262 panel failed");
  // init st7262 panel
  ESP_GOTO_ON_ERROR(esp_lcd_panel_init(*disp_panel), err, "",
                    "init st7262 panel failed");
  // set Orientation
  if (panel_cfg->orientation == orientation_landscape ||
      panel_cfg->orientation == orientation_landscape_invert) {
    ESP_GOTO_ON_ERROR(esp_lcd_panel_swap_xy(*disp_panel, true), err, "",
                      "swap st7262 panel failed");
  }
  if (panel_cfg->orientation == orientation_portrait_invert ||
      panel_cfg->orientation == orientation_landscape_invert) {
    ESP_GOTO_ON_ERROR(esp_lcd_panel_mirror(*disp_panel, true, true), err, "",
                      "mirror st7262 panel failed");
  }

  return ESP_OK;

err:
  if (lcd_panel) {
    if (panel_cfg->panel_dev_config.reset_gpio_num >= 0) {
      gpio_reset_pin(panel_cfg->panel_dev_config.reset_gpio_num);
    }
    free(lcd_panel);
  }
  esp3d_log_e("new st7796 panel failed err=%d", ret);
  return ret;
}

/**********************
 *   Static FUNCTIONS
 **********************/

/**
 * @brief Deletes an LCD panel.
 *
 * This function is used to delete an LCD panel structure and free the
 * associated memory.
 *
 * @param panel Pointer to the LCD panel structure to be deleted.
 * @return `ESP_OK` if the LCD panel is successfully deleted, or an error code
 * if an error occurred.
 */
static esp_err_t lcd_panel_del(esp_lcd_panel_t *panel) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_err_t ret = ESP_OK;
  if (GPIO_IS_VALID_GPIO(lcd_panel->reset_gpio_num)) {
    gpio_reset_pin(lcd_panel->reset_gpio_num);
  } else {
    ret = ESP_ERR_INVALID_ARG;
    esp3d_log_e("invalid GPIO pin");
  }

  if (ret == ESP_OK) {
    esp3d_log("del st7796 panel @%p", lcd_panel);
  } else {
    esp3d_log_e("failed to delete st7796 panel, err=%d", ret);
  }

  free(lcd_panel);
  return ret;
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
 * code indicating the cause of the failure.
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

  // Command Set Control - Enable Extension Command 2 part I
  esp_lcd_panel_io_tx_param(io, 0xF0, (uint8_t[]){0xC3}, 1);

  // Positive Gamma Control
  esp_lcd_panel_io_tx_param(
      io, 0xE0,
      (uint8_t[]){0xF0, 0x09, 0x0B, 0x06, 0x04, 0x15, 0x2F, 0x54, 0x42, 0x3C,
                  0x17, 0x14, 0x18, 0x1B},
      14);
  // Negative Gamma Control
  esp_lcd_panel_io_tx_param(
      io, 0xE1,
      (uint8_t[]){0xE0, 0x09, 0x0B, 0x06, 0x04, 0x03, 0x2B, 0x43, 0x42, 0x3B,
                  0x16, 0x14, 0x17, 0x1B},
      14);

  // Command Set Control - Disable Extension Command 2 part I
  esp_lcd_panel_io_tx_param(io, 0xF0, (uint8_t[]){0x3C}, 1);

  // Turn on display
  esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);

  return ESP_OK;
}

/**
 * @brief Draws a bitmap on the LCD panel.
 *
 * This function draws a bitmap on the LCD panel starting from the specified
 * coordinates (x_start, y_start) and ending at the specified coordinates
 * (x_end, y_end). The bitmap data is provided through the `color_data`
 * parameter.
 *
 * @param panel Pointer to the LCD panel structure.
 * @param x_start The starting x-coordinate of the bitmap.
 * @param y_start The starting y-coordinate of the bitmap.
 * @param x_end The ending x-coordinate of the bitmap.
 * @param y_end The ending y-coordinate of the bitmap.
 * @param color_data Pointer to the bitmap color data.
 *
 * @return `ESP_OK` if the bitmap was successfully drawn, or an error code if an
 * error occurred.
 */
static esp_err_t lcd_panel_draw_bitmap(esp_lcd_panel_t *panel, int x_start,
                                       int y_start, int x_end, int y_end,
                                       const void *color_data) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  if (!((x_start < x_end) && (y_start < y_end))) {
    esp3d_log_e("start position must be smaller than end position");
    return ESP_ERR_INVALID_ARG;
  }
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
 * This function is used to mirror the LCD panel horizontally and/or vertically.
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
 *
 * @return `ESP_OK` if the axes are successfully swapped, otherwise an error
 * code.
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
 * @brief Sets the gap between pixels on the LCD panel.
 *
 * This function sets the gap between pixels on the LCD panel for the specified
 * panel.
 *
 * @param panel Pointer to the LCD panel structure.
 * @param x_gap The horizontal gap between pixels.
 * @param y_gap The vertical gap between pixels.
 *
 * @return `ESP_OK` if the gap is set successfully, or an error code if an error
 * occurred.
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
 * This function is used to control the power state of the LCD panel.
 *
 * @param panel Pointer to the LCD panel structure.
 * @param off   Boolean value indicating whether to turn the display off (true)
 * or on (false).
 *
 * @return
 *     - ESP_OK if the operation is successful.
 *     - ESP_FAIL if there is an error in turning the display on or off.
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
