/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Created by luc lebosse - luc@tech-hunters - https://github.com/luc-github -
 * 2022-09-20
 */
#include "st7796.h"

#include <stdlib.h>
#include <sys/cdefs.h>

#include "driver/gpio.h"
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
  uint16_t width;   // Current screen width, it may change when apply to rotate
  uint16_t height;  // Current screen height, it may change when apply to rotate
  uint8_t dir;      // Current screen direction
  int x_gap;
  int y_gap;
  unsigned int bits_per_pixel;
  uint8_t madctl_val;  // MADCTL register addr:3600H
  uint8_t colmod_cal;  // Color Format: COLMOD register addr:3A00H
} lcd_panel_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static bool st7796_notify_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                                       esp_lcd_panel_io_event_data_t *edata,
                                       void *user_ctx);

static esp_err_t panel_st7796_del(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_init(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_draw_bitmap(esp_lcd_panel_t *panel, int x_start,
                                           int y_start, int x_end, int y_end,
                                           const void *color_data);
static esp_err_t panel_st7796_invert_color(esp_lcd_panel_t *panel,
                                            bool invert_color_data);
static esp_err_t panel_st7796_mirror(esp_lcd_panel_t *panel, bool mirror_x,
                                      bool mirror_y);
static esp_err_t panel_st7796_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_st7796_set_gap(esp_lcd_panel_t *panel, int x_gap,
                                       int y_gap);
static esp_err_t panel_st7796_disp_off(esp_lcd_panel_t *panel, bool off);

esp_err_t esp_lcd_new_panel_st7796(
    const esp_lcd_panel_io_handle_t io,
    const esp_i80_st7796_config_t *st7796_config,
    esp_lcd_panel_handle_t *ret_panel);


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initializes the st7796 display driver.
 *
 * This function initializes the st7796 display driver with the provided configuration.
 *
 * @param disp_st7796_cfg Pointer to the configuration structure for the st7796 display driver.
 * @param panel_handle Pointer to the handle of the LCD panel.
 * @param flush_ready_fn Pointer to the flush ready function.
 * @return `ESP_OK` if the initialization is successful, otherwise an error code.
 */
esp_err_t st7796_init(const esp_i80_st7796_config_t *disp_st7796_cfg, esp_lcd_panel_handle_t *panel_handle, void * flush_ready_fn) {
  if (*panel_handle != NULL || disp_st7796_cfg == NULL) {
    esp3d_log_e("st7796 bus already initialized");
    return ESP_FAIL;
  }
  esp3d_log("init st7796 bus");
  esp_lcd_i80_bus_handle_t i80_bus = NULL;
  esp3d_log("init lcd bus");
  ESP_ERROR_CHECK(
      esp_lcd_new_i80_bus(&(disp_st7796_cfg->bus_config), &i80_bus));
  if (i80_bus == NULL) {
    esp3d_log_e("init lcd i80 display bus failed");
  }

  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_i80_config_t io_config = disp_st7796_cfg->io_config;
  io_config.on_color_trans_done =
      st7796_notify_flush_ready;  // Callback invoked when color data
                                   // transfer has finished
  io_config.user_ctx = flush_ready_fn;   // User private data, passed directly to
                                   // on_color_trans_done’s user_ctx

  esp3d_log("init lcd panel");
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
  esp3d_log("init lcd panel st7796");
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_st7796(io_handle, (disp_st7796_cfg), panel_handle));

  esp_lcd_panel_reset(*panel_handle);  // LCD Reset
  esp_lcd_panel_init(*panel_handle);   // LCD init
  esp_lcd_panel_invert_color(*panel_handle, true);
 if (disp_st7796_cfg->orientation == orientation_landscape || disp_st7796_cfg->orientation == orientation_landscape_invert){
    panel_st7796_swap_xy(*panel_handle,true);
}

  return ESP_OK;
}


/**********************
 *   Static FUNCTIONS
 **********************/

/**
 * @brief Notifies when the flush operation is ready.
 *
 * This function is called to notify when the flush operation is ready for the st7796 LCD panel.
 *
 * @param panel_io The handle to the LCD panel I/O.
 * @param edata The event data associated with the flush operation.
 * @param user_ctx The user context passed to the function.
 * @return `true` if the notification was successful, `false` otherwise.
 */
static bool st7796_notify_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                                       esp_lcd_panel_io_event_data_t *edata,
                                       void *user_ctx) {
   void (*flush_ready)(void) = (void (*)(void))user_ctx;
  flush_ready();
  return false;
}

/**
 * @brief Initializes a new st7796 LCD panel.
 *
 * This function initializes a new st7796 LCD panel using the provided I/O handle and configuration.
 *
 * @param io The I/O handle for the LCD panel.
 * @param st7796_config The configuration for the st7796 LCD panel.
 * @param ret_panel Pointer to store the handle of the initialized LCD panel.
 * @return `ESP_OK` if the LCD panel is successfully initialized, or an error code if initialization fails.
 */
esp_err_t esp_lcd_new_panel_st7796(
    const esp_lcd_panel_io_handle_t io,
    const esp_i80_st7796_config_t *st7796_config,
    esp_lcd_panel_handle_t *ret_panel) {
  esp_err_t ret = ESP_OK;
  lcd_panel_t *st7796 = NULL;
  ESP_GOTO_ON_FALSE(io && st7796_config && ret_panel, ESP_ERR_INVALID_ARG,
                    err, "", "invalid argument");
  st7796 = calloc(1, sizeof(lcd_panel_t));
  ESP_GOTO_ON_FALSE(st7796, ESP_ERR_NO_MEM, err, "",
                    "no mem for st7796 panel");

  if (st7796_config->panel_config.reset_gpio_num >= 0) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << (st7796_config->panel_config.reset_gpio_num),
    };
    ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, "",
                      "configure GPIO for RST line failed");
  }


  switch (st7796_config->panel_config.rgb_ele_order) {
    case LCD_RGB_ELEMENT_ORDER_RGB:
      st7796->madctl_val = 0;
      break;
    case LCD_RGB_ELEMENT_ORDER_BGR:
      st7796->madctl_val |= LCD_CMD_BGR_BIT;
      break;
    default:
      ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "",
                        "unsupported rgb_ele_order");
      break;
  }

  // Color Format, REG:3A00H
  switch (st7796_config->panel_config.bits_per_pixel) {
    case 16:
      st7796->colmod_cal = 0x55;
      break;
    case 18:
      st7796->colmod_cal = 0x66;
      break;
    case 24:
      st7796->colmod_cal = 0x77;
      break;
    default:
      ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "",
                        "unsupported pixel width");
      break;
  }

  st7796->x_gap = 0;
  st7796->y_gap = 0;
  st7796->io = io;
  st7796->bits_per_pixel = st7796_config->panel_config.bits_per_pixel;
  st7796->reset_gpio_num = st7796_config->panel_config.reset_gpio_num;
  st7796->reset_level = st7796_config->panel_config.flags.reset_active_high;
  st7796->base.del = panel_st7796_del;
  st7796->base.reset = panel_st7796_reset;
  st7796->base.init = panel_st7796_init;
  st7796->base.draw_bitmap = panel_st7796_draw_bitmap;
  st7796->base.invert_color = panel_st7796_invert_color;
  st7796->base.set_gap = panel_st7796_set_gap;
  st7796->base.mirror = panel_st7796_mirror;
  st7796->base.swap_xy = panel_st7796_swap_xy;
  st7796->base.disp_on_off = panel_st7796_disp_off;
  *ret_panel = &(st7796->base);
  esp3d_log("new st7796 panel @%p", st7796);

  return ESP_OK;

err:
  if (st7796) {
    if (st7796_config->panel_config.reset_gpio_num >= 0) {
      gpio_reset_pin(st7796_config->panel_config.reset_gpio_num);
    }
    free(st7796);
  }
  return ret;
}

/**
 * @brief Deletes the st7796 panel.
 *
 * This function is responsible for deleting the st7796 panel.
 *
 * @param panel Pointer to the ESP LCD panel structure.
 * @return `ESP_OK` if the panel is successfully deleted, otherwise an error code.
 */
static esp_err_t panel_st7796_del(esp_lcd_panel_t *panel) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);

  if (st7796->reset_gpio_num >= 0) {
    gpio_reset_pin(st7796->reset_gpio_num);
  }
  esp3d_log("del st7796 panel @%p", st7796);
  free(st7796);
  return ESP_OK;
}

/**
 * @brief Resets the st7796 LCD panel.
 *
 * This function is responsible for resetting the st7796 LCD panel.
 *
 * @param panel Pointer to the esp_lcd_panel_t structure representing the LCD panel.
 * @return `ESP_OK` if the reset operation is successful, otherwise an error code.
 */
static esp_err_t panel_st7796_reset(esp_lcd_panel_t *panel) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = st7796->io;

  // perform hardware reset
  if (st7796->reset_gpio_num >= 0) {
    gpio_set_level(st7796->reset_gpio_num, st7796->reset_level);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(st7796->reset_gpio_num, !st7796->reset_level);
    vTaskDelay(pdMS_TO_TICKS(10));
  } else {
    // perform software reset
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET << 8, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(
        20));  // spec, wait at least 5m before sending new command
  }

  return ESP_OK;
}

/**
 * @brief Initializes the st7796 LCD panel.
 *
 * This function is responsible for initializing the st7796 LCD panel.
 *
 * @param panel Pointer to the esp_lcd_panel_t structure representing the LCD panel.
 * @return `ESP_OK` if the initialization is successful, otherwise an error code.
 */
static esp_err_t panel_st7796_init(esp_lcd_panel_t *panel) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = st7796->io;

  // SLEEP OUT
  esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  // scanning direction
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL,
                            (uint16_t[]){
                                st7796->madctl_val,
                            },
                            1);
  // Color Format
  esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD ,
                            (uint16_t[]){
                                st7796->colmod_cal,
                            },
                            1);
  // turn on display
  esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON , NULL, 0);

  esp3d_log("LCD=%dx%d dir=%d xgap=%d ygap=%d", st7796->width, st7796->height,
            st7796->dir, st7796->x_gap, st7796->y_gap);
  esp3d_log("madctl=0x%02X colmod=0x%02X", st7796->madctl_val,
            st7796->colmod_cal);

  return ESP_OK;
}

/**
 * @brief Draws a bitmap on the st7796 LCD panel.
 *
 * This function draws a bitmap on the st7796 LCD panel starting from the specified coordinates (x_start, y_start) and ending at the specified coordinates (x_end, y_end). The bitmap data is provided in the color_data parameter.
 *
 * @param panel The st7796 LCD panel.
 * @param x_start The starting x-coordinate of the bitmap.
 * @param y_start The starting y-coordinate of the bitmap.
 * @param x_end The ending x-coordinate of the bitmap.
 * @param y_end The ending y-coordinate of the bitmap.
 * @param color_data The bitmap data.
 * @return ESP_OK if the bitmap was successfully drawn, otherwise an error code.
 */
static esp_err_t panel_st7796_draw_bitmap(esp_lcd_panel_t *panel, int x_start,
                                           int y_start, int x_end, int y_end,
                                           const void *color_data) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);
  assert((x_start <= x_end) && (y_start <= y_end) &&
         "start position must be smaller than end position");
  esp_lcd_panel_io_handle_t io = st7796->io;

  x_start += st7796->x_gap;
  x_end += st7796->x_gap;
  y_start += st7796->y_gap;
  y_end += st7796->y_gap;

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
    size_t len = (x_end - x_start) * (y_end - y_start) * st7796->bits_per_pixel / 8;
    esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, len);

  return ESP_OK;
}

/**
 * @brief Inverts the color data of the st7796 LCD panel.
 *
 * This function is used to invert the color data of the st7796 LCD panel.
 *
 * @param panel The pointer to the ESP LCD panel structure.
 * @param invert_color_data A boolean value indicating whether to invert the color data.
 *                          Set to `true` to invert the color data, or `false` to keep it unchanged.
 * @return `ESP_OK` if the color data inversion is successful, or an error code if it fails.
 */
static esp_err_t panel_st7796_invert_color(esp_lcd_panel_t *panel,
                                            bool invert_color_data) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = st7796->io;
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
 * @brief Mirrors the st7796 LCD panel horizontally and/or vertically.
 *
 * This function is used to mirror the st7796 LCD panel either horizontally,
 * vertically, or both horizontally and vertically. The `mirror_x` parameter
 * determines whether the panel should be mirrored horizontally, while the
 * `mirror_y` parameter determines whether the panel should be mirrored
 * vertically.
 *
 * @param panel The st7796 LCD panel structure.
 * @param mirror_x Set to `true` to mirror the panel horizontally, or `false` to
 *                 leave it unmirrored.
 * @param mirror_y Set to `true` to mirror the panel vertically, or `false` to
 *                 leave it unmirrored.
 * @return `ESP_OK` if the panel was successfully mirrored, or an error code if
 *         an error occurred.
 */
static esp_err_t panel_st7796_mirror(esp_lcd_panel_t *panel, bool mirror_x,
                                      bool mirror_y) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = st7796->io;
  if (mirror_x) {
    st7796->madctl_val |= LCD_CMD_MX_BIT;
  } else {
    st7796->madctl_val &= ~LCD_CMD_MX_BIT;
  }
  if (mirror_y) {
    st7796->madctl_val |= LCD_CMD_MY_BIT;
  } else {
    st7796->madctl_val &= ~LCD_CMD_MY_BIT;
  }
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL,
                            (uint16_t[]){st7796->madctl_val}, 1);
  return ESP_OK;
}

/**
 * @brief Swaps the X and Y axes of the st7796 LCD panel.
 *
 * This function is used to swap the X and Y axes of the st7796 LCD panel.
 * If `swap_axes` is set to `true`, the X and Y axes will be swapped. Otherwise,
 * they will remain unchanged.
 *
 * @param panel Pointer to the st7796 LCD panel structure.
 * @param swap_axes Flag indicating whether to swap the X and Y axes.
 * @return `ESP_OK` if the operation is successful, otherwise an error code.
 */
static esp_err_t panel_st7796_swap_xy(esp_lcd_panel_t *panel, bool swap_axes) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = st7796->io;
  if (swap_axes) {
    st7796->madctl_val |= LCD_CMD_MV_BIT;
  } else {
    st7796->madctl_val &= ~LCD_CMD_MV_BIT;
  }
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL,
                            (uint16_t[]){st7796->madctl_val}, 1);
  return ESP_OK;
}

/**
 * @brief Sets the gap between pixels on the st7796 LCD panel.
 *
 * This function sets the gap between pixels on the st7796 LCD panel
 * specified by the `panel` parameter. The `x_gap` parameter represents
 * the horizontal gap between pixels, while the `y_gap` parameter represents
 * the vertical gap between pixels.
 *
 * @param panel Pointer to the st7796 LCD panel structure.
 * @param x_gap The horizontal gap between pixels.
 * @param y_gap The vertical gap between pixels.
 * @return `ESP_OK` if the gap is set successfully, otherwise an error code.
 */
static esp_err_t panel_st7796_set_gap(esp_lcd_panel_t *panel, int x_gap,
                                       int y_gap) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);
  st7796->x_gap = x_gap;
  st7796->y_gap = y_gap;
  return ESP_OK;
}

/**
 * @brief Turns the display on or off for the st7796 panel.
 *
 * This function is used to control the display power state of the st7796 panel.
 *
 * @param panel Pointer to the ESP LCD panel structure.
 * @param off   Boolean value indicating whether to turn off the display (true) or turn it on (false).
 *
 * @return
 *     - ESP_OK if the display power state was successfully changed.
 *     - ESP_ERR_INVALID_ARG if the panel pointer is NULL.
 *     - ESP_FAIL if there was an error communicating with the st7796 panel.
 */
static esp_err_t panel_st7796_disp_off(esp_lcd_panel_t *panel, bool off) {
  lcd_panel_t *st7796 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = st7796->io;
  int command = 0;
  if (off) {
    command = LCD_CMD_DISPOFF;
  } else {
    command = LCD_CMD_DISPON;
  }
  esp_lcd_panel_io_tx_param(io, command << 8, NULL, 0);
  return ESP_OK;
}

