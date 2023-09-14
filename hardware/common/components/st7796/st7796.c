/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <sys/cdefs.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include "esp3d_log.h"
#include "esp_check.h"
#include "st7796.h"

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
  uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
  uint8_t colmod_cal; // save current value of LCD_CMD_COLMOD register
} lcd_panel_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/

static esp_err_t lcd_panel_del(esp_lcd_panel_t *panel);
static esp_err_t lcd_panel_reset(esp_lcd_panel_t *panel);
static esp_err_t lcd_panel_init(esp_lcd_panel_t *panel);
static esp_err_t lcd_panel_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t lcd_panel_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t lcd_panel_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t lcd_panel_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t lcd_panel_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t lcd_panel_disp_on_off(esp_lcd_panel_t *panel, bool off);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_err_t esp_lcd_new_panel_st7796(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel) {
  esp_err_t ret = ESP_OK;
  lcd_panel_t *lcd_panel = NULL;
  ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, "", "invalid argument");
  lcd_panel = calloc(1, sizeof(lcd_panel_t));
  ESP_GOTO_ON_FALSE(lcd_panel, ESP_ERR_NO_MEM, err, "", "no mem for st7796 panel");

  if (panel_dev_config->reset_gpio_num >= 0) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
    };
    ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, "", "configure GPIO for RST line failed");
  }

  switch (panel_dev_config->color_space) {
    case ESP_LCD_COLOR_SPACE_RGB:
      lcd_panel->madctl_val = 0;
      break;
    case ESP_LCD_COLOR_SPACE_BGR:
      lcd_panel->madctl_val |= LCD_CMD_BGR_BIT;
      break;
    default:
      ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "", "unsupported color space");
      break;
  }

  switch (panel_dev_config->bits_per_pixel) {
    case 16:
      lcd_panel->colmod_cal = 0x55;
      break;
    case 18:
      lcd_panel->colmod_cal = 0x66;
      break;
    default:
      ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "", "unsupported pixel width");
      break;
  }

  lcd_panel->io = io;
  lcd_panel->bits_per_pixel = panel_dev_config->bits_per_pixel;
  lcd_panel->reset_gpio_num = panel_dev_config->reset_gpio_num;
  lcd_panel->reset_level = panel_dev_config->flags.reset_active_high;
  lcd_panel->base.del = lcd_panel_del;
  lcd_panel->base.reset = lcd_panel_reset;
  lcd_panel->base.init = lcd_panel_init;
  lcd_panel->base.draw_bitmap = lcd_panel_draw_bitmap;
  lcd_panel->base.invert_color = lcd_panel_invert_color;
  lcd_panel->base.set_gap = lcd_panel_set_gap;
  lcd_panel->base.mirror = lcd_panel_mirror;
  lcd_panel->base.swap_xy = lcd_panel_swap_xy;
  lcd_panel->base.disp_on_off = lcd_panel_disp_on_off;
  *ret_panel = &(lcd_panel->base);
  esp3d_log("new st7796 panel @%p", lcd_panel);

  return ESP_OK;

err:
  if (lcd_panel) {
    if (panel_dev_config->reset_gpio_num >= 0) {
      gpio_reset_pin(panel_dev_config->reset_gpio_num);
    }
    free(lcd_panel);
  }
  return ret;
}

/**********************
 *   Static FUNCTIONS
 **********************/

static esp_err_t lcd_panel_del(esp_lcd_panel_t *panel) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);

  if (lcd_panel->reset_gpio_num >= 0) {
    gpio_reset_pin(lcd_panel->reset_gpio_num);
  }
  esp3d_log("del st7796 panel @%p", lcd_panel);
  free(lcd_panel);
  return ESP_OK;
}

static esp_err_t lcd_panel_reset(esp_lcd_panel_t *panel) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;

  // perform hardware reset
  if (lcd_panel->reset_gpio_num >= 0) {
    gpio_set_level(lcd_panel->reset_gpio_num, lcd_panel->reset_level);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(lcd_panel->reset_gpio_num, !lcd_panel->reset_level);
    vTaskDelay(pdMS_TO_TICKS(10));
  } else { // perform software reset
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); // spec, wait at least 5 ms. before sending new command
  }

  return ESP_OK;
}

static esp_err_t lcd_panel_init(esp_lcd_panel_t *panel) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;

  // LCD goes into sleep mode and display will be turned off after power on reset, exit sleep mode first
  esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(100));

  // Memory Data Access Control
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
      lcd_panel->madctl_val,
  }, 1);
  // Interface Pixel Format
  esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, (uint8_t[]) {
      lcd_panel->colmod_cal,
  }, 1);

  // Command Set Control - Enable Extension Command 2 part I
  esp_lcd_panel_io_tx_param(io, 0xF0, (uint8_t[]){0xC3}, 1);

  // Positive Gamma Control
  esp_lcd_panel_io_tx_param(io, 0xE0, (uint8_t[]) {
      0xF0, 0x09, 0x0B, 0x06, 0x04, 0x15, 0x2F,
      0x54, 0x42, 0x3C, 0x17, 0x14, 0x18, 0x1B
  }, 14);
  // Negative Gamma Control
  esp_lcd_panel_io_tx_param(io, 0xE1, (uint8_t[]) {
      0xE0, 0x09, 0x0B, 0x06, 0x04, 0x03, 0x2B,
      0x43, 0x42, 0x3B, 0x16, 0x14, 0x17, 0x1B
  }, 14);
    
  // Command Set Control - Disable Extension Command 2 part I
  esp_lcd_panel_io_tx_param(io, 0xF0, (uint8_t[]){0x3C}, 1);

  // Turn on display
  esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);

  return ESP_OK;
}

static esp_err_t lcd_panel_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
  esp_lcd_panel_io_handle_t io = lcd_panel->io;

  x_start += lcd_panel->x_gap;
  x_end += lcd_panel->x_gap;
  y_start += lcd_panel->y_gap;
  y_end += lcd_panel->y_gap;

  // define an area of frame memory where MCU can access
  esp_lcd_panel_io_tx_param(io, LCD_CMD_CASET, (uint8_t[]) {
      (x_start >> 8) & 0xFF,
      x_start & 0xFF,
      ((x_end - 1) >> 8) & 0xFF,
      (x_end - 1) & 0xFF,
  }, 4);
  esp_lcd_panel_io_tx_param(io, LCD_CMD_RASET, (uint8_t[]) {
      (y_start >> 8) & 0xFF,
      y_start & 0xFF,
      ((y_end - 1) >> 8) & 0xFF,
      (y_end - 1) & 0xFF,
  }, 4);

  // transfer frame buffer
  size_t len = (x_end - x_start) * (y_end - y_start) * lcd_panel->bits_per_pixel / 8;
  esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, len);

  return ESP_OK;
}

static esp_err_t lcd_panel_invert_color(esp_lcd_panel_t *panel, bool invert_color_data) {
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

static esp_err_t lcd_panel_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y) {
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
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
    lcd_panel->madctl_val
  }, 1);
  return ESP_OK;
}

static esp_err_t lcd_panel_swap_xy(esp_lcd_panel_t *panel, bool swap_axes) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = lcd_panel->io;
  if (swap_axes) {
    lcd_panel->madctl_val |= LCD_CMD_MV_BIT;
  } else {
    lcd_panel->madctl_val &= ~LCD_CMD_MV_BIT;
  }
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
    lcd_panel->madctl_val
  }, 1);
  return ESP_OK;
}

static esp_err_t lcd_panel_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);
  lcd_panel->x_gap = x_gap;
  lcd_panel->y_gap = y_gap;
  return ESP_OK;
}

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
