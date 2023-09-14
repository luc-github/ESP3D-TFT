/**
 * @file ili9341.c
 */

#include <stdlib.h>
#include <sys/cdefs.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include "esp3d_log.h"
#include "esp_check.h"
#include "ili9341.h"

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

/* The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
  uint8_t cmd;
  uint8_t data[16];
  uint8_t num_bytes; // Number of data bytes; 0xFF = end of cmds.
} lcd_cmd_t;

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

esp_err_t esp_lcd_new_panel_ili9341(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel) {
  esp_err_t ret = ESP_OK;
  lcd_panel_t *lcd_panel = NULL;
  ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, "", "invalid argument");
  lcd_panel = calloc(1, sizeof(lcd_panel_t));
  ESP_GOTO_ON_FALSE(lcd_panel, ESP_ERR_NO_MEM, err, "", "no mem for ili9341 panel");

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
  esp3d_log("new ili9341 panel @%p", lcd_panel);

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
 *   STATIC FUNCTIONS
 **********************/

static esp_err_t lcd_panel_del(esp_lcd_panel_t *panel) {
  lcd_panel_t *lcd_panel = __containerof(panel, lcd_panel_t, base);

  if (lcd_panel->reset_gpio_num >= 0) {
    gpio_reset_pin(lcd_panel->reset_gpio_num);
  }
  esp3d_log("del ili9341 panel @%p", lcd_panel);
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
    {0xB1, {0x00, 0x1B}, 2},                    // Frame Rate Control (In Normal Mode/Full Colors)
    {0xF2, {0x08}, 1},                          // Enable 3G (3 gamma control)
    {0x26, {0x01}, 1},                          // Gamma Set
    {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15}, // Positive Gamme Correction
    {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15}, // Negative Gamme Correction
    {0xB7, {0x07}, 1},                          // Entry Mode Set
    {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},        // Display Function Control
    {0, {0}, 0xFF},                             // NOP
  };

  // Send all the vendor init commands
  uint8_t i = 0;
  while (vendor_init_cmds[i].num_bytes != 0xFF) {
    esp_lcd_panel_io_tx_param(io, vendor_init_cmds[i].cmd, vendor_init_cmds[i].data, vendor_init_cmds[i].num_bytes);
    i++;
  }

  // Memory Data Access Control
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
      lcd_panel->madctl_val,
  }, 1);
  // Interface Pixel Format
  esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, (uint8_t[]) {
      lcd_panel->colmod_cal,
  }, 1);

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
