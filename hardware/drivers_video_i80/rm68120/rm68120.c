/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Created by luc lebosse - luc@tech-hunters - https://github.com/luc-github -
 * 2022-09-20
 */
#include "rm68120.h"

#include <stdlib.h>
#include <sys/cdefs.h>

#include "driver/gpio.h"
#include "esp3d_log.h"
#include "esp_check.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Define all screen direction */
typedef enum {
  /* @---> X
     |
     Y
  */
  SCR_DIR_LRTB, /**< From left to right then from top to bottom, this consider
                   as the original direction of the screen */

  /*  Y
      |
      @---> X
  */
  SCR_DIR_LRBT, /**< From left to right then from bottom to top */

  /* X <---@
           |
           Y
  */
  SCR_DIR_RLTB, /**< From right to left then from top to bottom */

  /*       Y
           |
     X <---@
  */
  SCR_DIR_RLBT, /**< From right to left then from bottom to top */

  /* @---> Y
     |
     X
  */
  SCR_DIR_TBLR, /**< From top to bottom then from left to right */

  /*  X
      |
      @---> Y
  */
  SCR_DIR_BTLR, /**< From bottom to top then from left to right */

  /* Y <---@
           |
           X
  */
  SCR_DIR_TBRL, /**< From top to bottom then from right to left */

  /*       X
           |
     Y <---@
  */
  SCR_DIR_BTRL, /**< From bottom to top then from right to left */

  SCR_DIR_MAX,
} scr_dir_t;

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

static bool rm68120_notify_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                                       esp_lcd_panel_io_event_data_t *edata,
                                       void *user_ctx);
static esp_err_t panel_rm68120_del(esp_lcd_panel_t *panel);
static esp_err_t panel_rm68120_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_rm68120_init(esp_lcd_panel_t *panel);
static esp_err_t panel_rm68120_draw_bitmap(esp_lcd_panel_t *panel, int x_start,
                                           int y_start, int x_end, int y_end,
                                           const void *color_data);
static esp_err_t panel_rm68120_invert_color(esp_lcd_panel_t *panel,
                                            bool invert_color_data);
static esp_err_t panel_rm68120_mirror(esp_lcd_panel_t *panel, bool mirror_x,
                                      bool mirror_y);
static esp_err_t panel_rm68120_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_rm68120_set_gap(esp_lcd_panel_t *panel, int x_gap,
                                       int y_gap);
static esp_err_t panel_rm68120_disp_off(esp_lcd_panel_t *panel, bool off);
static void rm68120_reg_config(esp_lcd_panel_t *panel);
esp_err_t esp_lcd_new_panel_rm68120(
    const esp_lcd_panel_io_handle_t io,
    const esp_lcd_panel_dev_config_t *panel_dev_config,
    esp_lcd_panel_handle_t *ret_panel);
void rm68120_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                   lv_color_t *color_p);
esp_lcd_panel_handle_t panel_handle = NULL;

esp_lcd_panel_handle_t *rm68120_panel_handle() { return &panel_handle; }

static bool rm68120_notify_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                                       esp_lcd_panel_io_event_data_t *edata,
                                       void *user_ctx) {
  lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
  lv_disp_flush_ready(disp_driver);
  return false;
}

void rm68120_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                   lv_color_t *color_p) {
  esp_lcd_panel_handle_t panel_handle =
      (esp_lcd_panel_handle_t)disp_drv->user_data;

  esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1,
                            area->y2 + 1, color_p);
}

esp_err_t rm68120_init(lv_disp_drv_t *disp_drv) {
  if (panel_handle != NULL) {
    esp3d_log_e("rm68120 bus already initialized");
    return ESP_FAIL;
  }
  esp3d_log("init rm68120 bus");
  esp_lcd_i80_bus_handle_t i80_bus = NULL;
  esp_lcd_i80_bus_config_t bus_config = {
      .clk_src = LCD_CLK_SRC_DEFAULT,
      .dc_gpio_num = DISP_RS_PIN,
      .wr_gpio_num = DISP_WR_PIN,
      .data_gpio_nums =
          {
              DISP_D00_PIN,
              DISP_D01_PIN,
              DISP_D02_PIN,
              DISP_D03_PIN,
              DISP_D04_PIN,
              DISP_D05_PIN,
              DISP_D06_PIN,
              DISP_D07_PIN,
              DISP_D08_PIN,
              DISP_D09_PIN,
              DISP_D10_PIN,
              DISP_D11_PIN,
              DISP_D12_PIN,
              DISP_D13_PIN,
              DISP_D14_PIN,
              DISP_D15_PIN,
          },
      .bus_width = DISP_BITS_WIDTH,
      .max_transfer_bytes = DISP_BUF_SIZE * sizeof(uint16_t),
      .psram_trans_align = 64,
      .sram_trans_align = 4,
  };
  esp3d_log("init lcd bus");
  ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));
  if (i80_bus == NULL) {
    esp3d_log_e("init lcd i80 display bus failed");
  }

  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_i80_config_t io_config = {
      .cs_gpio_num = DISP_CS_PIN,
      .pclk_hz = DISP_CLK_FREQ,
      .trans_queue_depth = 10,
      .dc_levels =
          {
              .dc_idle_level = 0,
              .dc_cmd_level = 0,
              .dc_dummy_level = 0,
              .dc_data_level = 1,
          },
      .on_color_trans_done =
          rm68120_notify_flush_ready,  // Callback invoked when color data
                                       // transfer has finished
      .user_ctx = disp_drv,            // User private data, passed directly to
                                       // on_color_trans_done’s user_ctx
      .lcd_cmd_bits = DISP_CMD_BITS_WIDTH,
      .lcd_param_bits = DISP_PARAM_BITS_WIDTH,
  };
  esp3d_log("init lcd panel");
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = DISP_RST_PIN,
      .color_space = ESP_LCD_COLOR_SPACE_RGB,
      .bits_per_pixel = 16,
  };
  esp3d_log("init lcd panel rm68120");
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_rm68120(io_handle, &panel_config, &panel_handle));

  esp_lcd_panel_reset(panel_handle);  // LCD Reset
  esp_lcd_panel_init(panel_handle);   // LCD init

  return ESP_OK;
}

esp_err_t esp_lcd_new_panel_rm68120(
    const esp_lcd_panel_io_handle_t io,
    const esp_lcd_panel_dev_config_t *panel_dev_config,
    esp_lcd_panel_handle_t *ret_panel) {
  esp_err_t ret = ESP_OK;
  lcd_panel_t *rm68120 = NULL;
  ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG,
                    err, "", "invalid argument");
  rm68120 = calloc(1, sizeof(lcd_panel_t));
  ESP_GOTO_ON_FALSE(rm68120, ESP_ERR_NO_MEM, err, "",
                    "no mem for rm68120 panel");

  if (panel_dev_config->reset_gpio_num >= 0) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
    };
    ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, "",
                      "configure GPIO for RST line failed");
  }

  // scanning direction
#if DISP_DIRECTION_LANDSCAPE == 1  // landscape mode
  rm68120->dir = SCR_DIR_LRTB;
  rm68120->madctl_val = 0x60;
#else  // portrait mode
  rm68120->dir = SCR_DIR_TBLR;
  rm68120->madctl_val = 0x00;
#endif

  switch (rm68120->dir) {
    case SCR_DIR_LRTB:
      rm68120->width = DISP_HOR_RES_MAX;
      rm68120->height = DISP_VER_RES_MAX;
      break;
    case SCR_DIR_TBLR:
      rm68120->width = DISP_HOR_RES_MAX;
      rm68120->height = DISP_VER_RES_MAX;
      break;
    default:
      esp3d_log_e("undefine, do not use!!!!");
      break;
  }

  switch (panel_dev_config->color_space) {
    case ESP_LCD_COLOR_SPACE_RGB:
      rm68120->madctl_val &= ((~LCD_CMD_BGR_BIT) & 0xFF);
      break;
    case ESP_LCD_COLOR_SPACE_BGR:
      rm68120->madctl_val |= LCD_CMD_BGR_BIT;
      break;
    default:
      ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "",
                        "unsupported color space");
      break;
  }

  // Color Format, REG:3A00H
  switch (panel_dev_config->bits_per_pixel) {
    case 16:
      rm68120->colmod_cal = 0x55;
      break;
    case 18:
      rm68120->colmod_cal = 0x66;
      break;
    case 24:
      rm68120->colmod_cal = 0x77;
      break;
    default:
      ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "",
                        "unsupported pixel width");
      break;
  }

  rm68120->x_gap = 0;
  rm68120->y_gap = 0;
  rm68120->io = io;
  rm68120->bits_per_pixel = panel_dev_config->bits_per_pixel;
  rm68120->reset_gpio_num = panel_dev_config->reset_gpio_num;
  rm68120->reset_level = panel_dev_config->flags.reset_active_high;
  rm68120->base.del = panel_rm68120_del;
  rm68120->base.reset = panel_rm68120_reset;
  rm68120->base.init = panel_rm68120_init;
  rm68120->base.draw_bitmap = panel_rm68120_draw_bitmap;
  rm68120->base.invert_color = panel_rm68120_invert_color;
  rm68120->base.set_gap = panel_rm68120_set_gap;
  rm68120->base.mirror = panel_rm68120_mirror;
  rm68120->base.swap_xy = panel_rm68120_swap_xy;
  rm68120->base.disp_on_off = panel_rm68120_disp_off;
  *ret_panel = &(rm68120->base);
  esp3d_log("new rm68120 panel @%p", rm68120);

  return ESP_OK;

err:
  if (rm68120) {
    if (panel_dev_config->reset_gpio_num >= 0) {
      gpio_reset_pin(panel_dev_config->reset_gpio_num);
    }
    free(rm68120);
  }
  return ret;
}

static esp_err_t panel_rm68120_del(esp_lcd_panel_t *panel) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);

  if (rm68120->reset_gpio_num >= 0) {
    gpio_reset_pin(rm68120->reset_gpio_num);
  }
  esp3d_log("del rm68120 panel @%p", rm68120);
  free(rm68120);
  return ESP_OK;
}

static esp_err_t panel_rm68120_reset(esp_lcd_panel_t *panel) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = rm68120->io;

  // perform hardware reset
  if (rm68120->reset_gpio_num >= 0) {
    gpio_set_level(rm68120->reset_gpio_num, rm68120->reset_level);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(rm68120->reset_gpio_num, !rm68120->reset_level);
    vTaskDelay(pdMS_TO_TICKS(20));
  } else {
    // perform software reset
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET << 8, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(
        20));  // spec, wait at least 5m before sending new command
  }

  return ESP_OK;
}

static esp_err_t panel_rm68120_init(esp_lcd_panel_t *panel) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = rm68120->io;

  // register config
  rm68120_reg_config(panel);
  // SLEEP OUT
  esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT << 8, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  // scanning direction
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL << 8,
                            (uint16_t[]){
                                rm68120->madctl_val,
                            },
                            2);
  // Color Format
  esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD << 8,
                            (uint16_t[]){
                                rm68120->colmod_cal,
                            },
                            2);
  // turn on display
  esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON << 8, NULL, 0);

  esp3d_log("LCD=%dx%d dir=%d xgap=%d ygap=%d", rm68120->width, rm68120->height,
            rm68120->dir, rm68120->x_gap, rm68120->y_gap);
  esp3d_log("madctl=0x%02X colmod=0x%02X", rm68120->madctl_val,
            rm68120->colmod_cal);

  return ESP_OK;
}

/* * (x_start-x_end)*(y_start-y_end)= MAX(32640) * */
static esp_err_t panel_rm68120_draw_bitmap(esp_lcd_panel_t *panel, int x_start,
                                           int y_start, int x_end, int y_end,
                                           const void *color_data) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  assert((x_start <= x_end) && (y_start <= y_end) &&
         "start position must be smaller than end position");
  esp_lcd_panel_io_handle_t io = rm68120->io;

  x_start += rm68120->x_gap;
  x_end += rm68120->x_gap;
  y_start += rm68120->y_gap;
  y_end += rm68120->y_gap;

  // define an area of frame memory where MCU can access
  esp_lcd_panel_io_tx_param(io, (LCD_CMD_CASET << 8) + 0,
                            (uint16_t[]){
                                (x_start >> 8) & 0xFF,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, (LCD_CMD_CASET << 8) + 1,
                            (uint16_t[]){
                                x_start & 0xFF,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, (LCD_CMD_CASET << 8) + 2,
                            (uint16_t[]){
                                ((x_end - 1) >> 8) & 0xFF,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, (LCD_CMD_CASET << 8) + 3,
                            (uint16_t[]){
                                (x_end - 1) & 0xFF,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, (LCD_CMD_RASET << 8) + 0,
                            (uint16_t[]){
                                (y_start >> 8) & 0xFF,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, (LCD_CMD_RASET << 8) + 1,
                            (uint16_t[]){
                                y_start & 0xFF,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, (LCD_CMD_RASET << 8) + 2,
                            (uint16_t[]){
                                ((y_end - 1) >> 8) & 0xFF,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, (LCD_CMD_RASET << 8) + 3,
                            (uint16_t[]){
                                (y_end - 1) & 0xFF,
                            },
                            2);
  // transfer frame buffer
  size_t len =
      (x_end - x_start) * (y_end - y_start) * rm68120->bits_per_pixel / 8;
  esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR << 8, color_data, len);

  return ESP_OK;
}

static esp_err_t panel_rm68120_invert_color(esp_lcd_panel_t *panel,
                                            bool invert_color_data) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = rm68120->io;
  int command = 0;
  if (invert_color_data) {
    command = LCD_CMD_INVON;
  } else {
    command = LCD_CMD_INVOFF;
  }
  esp_lcd_panel_io_tx_param(io, command << 8, NULL, 0);
  return ESP_OK;
}

static esp_err_t panel_rm68120_mirror(esp_lcd_panel_t *panel, bool mirror_x,
                                      bool mirror_y) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = rm68120->io;
  if (mirror_x) {
    rm68120->madctl_val |= LCD_CMD_MX_BIT;
  } else {
    rm68120->madctl_val &= ~LCD_CMD_MX_BIT;
  }
  if (mirror_y) {
    rm68120->madctl_val |= LCD_CMD_MY_BIT;
  } else {
    rm68120->madctl_val &= ~LCD_CMD_MY_BIT;
  }
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL << 8,
                            (uint16_t[]){rm68120->madctl_val}, 2);
  return ESP_OK;
}

static esp_err_t panel_rm68120_swap_xy(esp_lcd_panel_t *panel, bool swap_axes) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = rm68120->io;
  if (swap_axes) {
    rm68120->madctl_val |= LCD_CMD_MV_BIT;
  } else {
    rm68120->madctl_val &= ~LCD_CMD_MV_BIT;
  }
  esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL << 8,
                            (uint16_t[]){rm68120->madctl_val}, 2);
  return ESP_OK;
}

static esp_err_t panel_rm68120_set_gap(esp_lcd_panel_t *panel, int x_gap,
                                       int y_gap) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  rm68120->x_gap = x_gap;
  rm68120->y_gap = y_gap;
  return ESP_OK;
}

static esp_err_t panel_rm68120_disp_off(esp_lcd_panel_t *panel, bool off) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = rm68120->io;
  int command = 0;
  if (off) {
    command = LCD_CMD_DISPOFF;
  } else {
    command = LCD_CMD_DISPON;
  }
  esp_lcd_panel_io_tx_param(io, command << 8, NULL, 0);
  return ESP_OK;
}

static void rm68120_reg_config(esp_lcd_panel_t *panel) {
  lcd_panel_t *rm68120 = __containerof(panel, lcd_panel_t, base);
  esp_lcd_panel_io_handle_t io = rm68120->io;

  // ENABLE PAGE 1
  esp_lcd_panel_io_tx_param(io, 0xF000,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF001,
                            (uint16_t[]){
                                0xAA,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF002,
                            (uint16_t[]){
                                0x52,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF003,
                            (uint16_t[]){
                                0x08,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF004,
                            (uint16_t[]){
                                0x01,
                            },
                            2);

  // GAMMA SETING  RED
  esp_lcd_panel_io_tx_param(io, 0xD100,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD101,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD102,
                            (uint16_t[]){
                                0x1b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD103,
                            (uint16_t[]){
                                0x44,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD104,
                            (uint16_t[]){
                                0x62,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD105,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD106,
                            (uint16_t[]){
                                0x7b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD107,
                            (uint16_t[]){
                                0xa1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD108,
                            (uint16_t[]){
                                0xc0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD109,
                            (uint16_t[]){
                                0xee,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD10A,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD10B,
                            (uint16_t[]){
                                0x10,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD10C,
                            (uint16_t[]){
                                0x2c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD10D,
                            (uint16_t[]){
                                0x43,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD10E,
                            (uint16_t[]){
                                0x57,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD10F,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD110,
                            (uint16_t[]){
                                0x68,
                            },
                            2);

  esp_lcd_panel_io_tx_param(io, 0xD111,
                            (uint16_t[]){
                                0x78,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD112,
                            (uint16_t[]){
                                0x87,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD113,
                            (uint16_t[]){
                                0x94,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD114,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD115,
                            (uint16_t[]){
                                0xa0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD116,
                            (uint16_t[]){
                                0xac,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD117,
                            (uint16_t[]){
                                0xb6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD118,
                            (uint16_t[]){
                                0xc1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD119,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD11A,
                            (uint16_t[]){
                                0xcb,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD11B,
                            (uint16_t[]){
                                0xcd,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD11C,
                            (uint16_t[]){
                                0xd6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD11D,
                            (uint16_t[]){
                                0xdf,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD11E,
                            (uint16_t[]){
                                0x95,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD11F,
                            (uint16_t[]){
                                0xe8,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD120,
                            (uint16_t[]){
                                0xf1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD121,
                            (uint16_t[]){
                                0xfa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD122,
                            (uint16_t[]){
                                0x02,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD123,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD124,
                            (uint16_t[]){
                                0x0b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD125,
                            (uint16_t[]){
                                0x13,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD126,
                            (uint16_t[]){
                                0x1d,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD127,
                            (uint16_t[]){
                                0x26,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD128,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD129,
                            (uint16_t[]){
                                0x30,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD12A,
                            (uint16_t[]){
                                0x3c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD12B,
                            (uint16_t[]){
                                0x4A,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD12C,
                            (uint16_t[]){
                                0x63,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD12D,
                            (uint16_t[]){
                                0xea,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD12E,
                            (uint16_t[]){
                                0x79,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD12F,
                            (uint16_t[]){
                                0xa6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD130,
                            (uint16_t[]){
                                0xd0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD131,
                            (uint16_t[]){
                                0x20,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD132,
                            (uint16_t[]){
                                0x0f,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD133,
                            (uint16_t[]){
                                0x8e,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD134,
                            (uint16_t[]){
                                0xff,
                            },
                            2);

  // GAMMA SETING GREEN
  esp_lcd_panel_io_tx_param(io, 0xD200,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD201,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD202,
                            (uint16_t[]){
                                0x1b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD203,
                            (uint16_t[]){
                                0x44,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD204,
                            (uint16_t[]){
                                0x62,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD205,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD206,
                            (uint16_t[]){
                                0x7b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD207,
                            (uint16_t[]){
                                0xa1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD208,
                            (uint16_t[]){
                                0xc0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD209,
                            (uint16_t[]){
                                0xee,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD20A,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD20B,
                            (uint16_t[]){
                                0x10,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD20C,
                            (uint16_t[]){
                                0x2c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD20D,
                            (uint16_t[]){
                                0x43,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD20E,
                            (uint16_t[]){
                                0x57,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD20F,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD210,
                            (uint16_t[]){
                                0x68,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD211,
                            (uint16_t[]){
                                0x78,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD212,
                            (uint16_t[]){
                                0x87,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD213,
                            (uint16_t[]){
                                0x94,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD214,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD215,
                            (uint16_t[]){
                                0xa0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD216,
                            (uint16_t[]){
                                0xac,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD217,
                            (uint16_t[]){
                                0xb6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD218,
                            (uint16_t[]){
                                0xc1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD219,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD21A,
                            (uint16_t[]){
                                0xcb,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD21B,
                            (uint16_t[]){
                                0xcd,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD21C,
                            (uint16_t[]){
                                0xd6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD21D,
                            (uint16_t[]){
                                0xdf,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD21E,
                            (uint16_t[]){
                                0x95,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD21F,
                            (uint16_t[]){
                                0xe8,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD220,
                            (uint16_t[]){
                                0xf1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD221,
                            (uint16_t[]){
                                0xfa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD222,
                            (uint16_t[]){
                                0x02,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD223,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD224,
                            (uint16_t[]){
                                0x0b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD225,
                            (uint16_t[]){
                                0x13,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD226,
                            (uint16_t[]){
                                0x1d,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD227,
                            (uint16_t[]){
                                0x26,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD228,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD229,
                            (uint16_t[]){
                                0x30,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD22A,
                            (uint16_t[]){
                                0x3c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD22B,
                            (uint16_t[]){
                                0x4a,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD22C,
                            (uint16_t[]){
                                0x63,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD22D,
                            (uint16_t[]){
                                0xea,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD22E,
                            (uint16_t[]){
                                0x79,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD22F,
                            (uint16_t[]){
                                0xa6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD230,
                            (uint16_t[]){
                                0xd0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD231,
                            (uint16_t[]){
                                0x20,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD232,
                            (uint16_t[]){
                                0x0f,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD233,
                            (uint16_t[]){
                                0x8e,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD234,
                            (uint16_t[]){
                                0xff,
                            },
                            2);

  // GAMMA SETING BLUE
  esp_lcd_panel_io_tx_param(io, 0xD300,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD301,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD302,
                            (uint16_t[]){
                                0x1b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD303,
                            (uint16_t[]){
                                0x44,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD304,
                            (uint16_t[]){
                                0x62,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD305,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD306,
                            (uint16_t[]){
                                0x7b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD307,
                            (uint16_t[]){
                                0xa1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD308,
                            (uint16_t[]){
                                0xc0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD309,
                            (uint16_t[]){
                                0xee,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD30A,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD30B,
                            (uint16_t[]){
                                0x10,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD30C,
                            (uint16_t[]){
                                0x2c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD30D,
                            (uint16_t[]){
                                0x43,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD30E,
                            (uint16_t[]){
                                0x57,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD30F,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD310,
                            (uint16_t[]){
                                0x68,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD311,
                            (uint16_t[]){
                                0x78,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD312,
                            (uint16_t[]){
                                0x87,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD313,
                            (uint16_t[]){
                                0x94,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD314,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD315,
                            (uint16_t[]){
                                0xa0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD316,
                            (uint16_t[]){
                                0xac,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD317,
                            (uint16_t[]){
                                0xb6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD318,
                            (uint16_t[]){
                                0xc1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD319,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD31A,
                            (uint16_t[]){
                                0xcb,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD31B,
                            (uint16_t[]){
                                0xcd,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD31C,
                            (uint16_t[]){
                                0xd6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD31D,
                            (uint16_t[]){
                                0xdf,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD31E,
                            (uint16_t[]){
                                0x95,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD31F,
                            (uint16_t[]){
                                0xe8,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD320,
                            (uint16_t[]){
                                0xf1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD321,
                            (uint16_t[]){
                                0xfa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD322,
                            (uint16_t[]){
                                0x02,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD323,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD324,
                            (uint16_t[]){
                                0x0b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD325,
                            (uint16_t[]){
                                0x13,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD326,
                            (uint16_t[]){
                                0x1d,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD327,
                            (uint16_t[]){
                                0x26,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD328,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD329,
                            (uint16_t[]){
                                0x30,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD32A,
                            (uint16_t[]){
                                0x3c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD32B,
                            (uint16_t[]){
                                0x4A,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD32C,
                            (uint16_t[]){
                                0x63,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD32D,
                            (uint16_t[]){
                                0xea,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD32E,
                            (uint16_t[]){
                                0x79,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD32F,
                            (uint16_t[]){
                                0xa6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD330,
                            (uint16_t[]){
                                0xd0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD331,
                            (uint16_t[]){
                                0x20,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD332,
                            (uint16_t[]){
                                0x0f,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD333,
                            (uint16_t[]){
                                0x8e,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD334,
                            (uint16_t[]){
                                0xff,
                            },
                            2);

  // GAMMA SETING  RED
  esp_lcd_panel_io_tx_param(io, 0xD400,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD401,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD402,
                            (uint16_t[]){
                                0x1b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD403,
                            (uint16_t[]){
                                0x44,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD404,
                            (uint16_t[]){
                                0x62,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD405,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD406,
                            (uint16_t[]){
                                0x7b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD407,
                            (uint16_t[]){
                                0xa1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD408,
                            (uint16_t[]){
                                0xc0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD409,
                            (uint16_t[]){
                                0xee,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD40A,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD40B,
                            (uint16_t[]){
                                0x10,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD40C,
                            (uint16_t[]){
                                0x2c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD40D,
                            (uint16_t[]){
                                0x43,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD40E,
                            (uint16_t[]){
                                0x57,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD40F,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD410,
                            (uint16_t[]){
                                0x68,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD411,
                            (uint16_t[]){
                                0x78,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD412,
                            (uint16_t[]){
                                0x87,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD413,
                            (uint16_t[]){
                                0x94,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD414,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD415,
                            (uint16_t[]){
                                0xa0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD416,
                            (uint16_t[]){
                                0xac,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD417,
                            (uint16_t[]){
                                0xb6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD418,
                            (uint16_t[]){
                                0xc1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD419,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD41A,
                            (uint16_t[]){
                                0xcb,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD41B,
                            (uint16_t[]){
                                0xcd,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD41C,
                            (uint16_t[]){
                                0xd6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD41D,
                            (uint16_t[]){
                                0xdf,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD41E,
                            (uint16_t[]){
                                0x95,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD41F,
                            (uint16_t[]){
                                0xe8,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD420,
                            (uint16_t[]){
                                0xf1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD421,
                            (uint16_t[]){
                                0xfa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD422,
                            (uint16_t[]){
                                0x02,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD423,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD424,
                            (uint16_t[]){
                                0x0b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD425,
                            (uint16_t[]){
                                0x13,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD426,
                            (uint16_t[]){
                                0x1d,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD427,
                            (uint16_t[]){
                                0x26,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD428,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD429,
                            (uint16_t[]){
                                0x30,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD42A,
                            (uint16_t[]){
                                0x3c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD42B,
                            (uint16_t[]){
                                0x4A,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD42C,
                            (uint16_t[]){
                                0x63,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD42D,
                            (uint16_t[]){
                                0xea,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD42E,
                            (uint16_t[]){
                                0x79,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD42F,
                            (uint16_t[]){
                                0xa6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD430,
                            (uint16_t[]){
                                0xd0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD431,
                            (uint16_t[]){
                                0x20,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD432,
                            (uint16_t[]){
                                0x0f,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD433,
                            (uint16_t[]){
                                0x8e,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD434,
                            (uint16_t[]){
                                0xff,
                            },
                            2);

  // GAMMA SETING GREEN
  esp_lcd_panel_io_tx_param(io, 0xD500,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD501,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD502,
                            (uint16_t[]){
                                0x1b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD503,
                            (uint16_t[]){
                                0x44,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD504,
                            (uint16_t[]){
                                0x62,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD505,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD506,
                            (uint16_t[]){
                                0x7b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD507,
                            (uint16_t[]){
                                0xa1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD508,
                            (uint16_t[]){
                                0xc0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD509,
                            (uint16_t[]){
                                0xee,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD50A,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD50B,
                            (uint16_t[]){
                                0x10,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD50C,
                            (uint16_t[]){
                                0x2c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD50D,
                            (uint16_t[]){
                                0x43,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD50E,
                            (uint16_t[]){
                                0x57,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD50F,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD510,
                            (uint16_t[]){
                                0x68,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD511,
                            (uint16_t[]){
                                0x78,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD512,
                            (uint16_t[]){
                                0x87,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD513,
                            (uint16_t[]){
                                0x94,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD514,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD515,
                            (uint16_t[]){
                                0xa0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD516,
                            (uint16_t[]){
                                0xac,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD517,
                            (uint16_t[]){
                                0xb6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD518,
                            (uint16_t[]){
                                0xc1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD519,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD51A,
                            (uint16_t[]){
                                0xcb,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD51B,
                            (uint16_t[]){
                                0xcd,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD51C,
                            (uint16_t[]){
                                0xd6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD51D,
                            (uint16_t[]){
                                0xdf,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD51E,
                            (uint16_t[]){
                                0x95,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD51F,
                            (uint16_t[]){
                                0xe8,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD520,
                            (uint16_t[]){
                                0xf1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD521,
                            (uint16_t[]){
                                0xfa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD522,
                            (uint16_t[]){
                                0x02,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD523,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD524,
                            (uint16_t[]){
                                0x0b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD525,
                            (uint16_t[]){
                                0x13,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD526,
                            (uint16_t[]){
                                0x1d,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD527,
                            (uint16_t[]){
                                0x26,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD528,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD529,
                            (uint16_t[]){
                                0x30,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD52A,
                            (uint16_t[]){
                                0x3c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD52B,
                            (uint16_t[]){
                                0x4a,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD52C,
                            (uint16_t[]){
                                0x63,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD52D,
                            (uint16_t[]){
                                0xea,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD52E,
                            (uint16_t[]){
                                0x79,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD52F,
                            (uint16_t[]){
                                0xa6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD530,
                            (uint16_t[]){
                                0xd0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD531,
                            (uint16_t[]){
                                0x20,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD532,
                            (uint16_t[]){
                                0x0f,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD533,
                            (uint16_t[]){
                                0x8e,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD534,
                            (uint16_t[]){
                                0xff,
                            },
                            2);

  // GAMMA SETING BLUE
  esp_lcd_panel_io_tx_param(io, 0xD600,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD601,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD602,
                            (uint16_t[]){
                                0x1b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD603,
                            (uint16_t[]){
                                0x44,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD604,
                            (uint16_t[]){
                                0x62,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD605,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD606,
                            (uint16_t[]){
                                0x7b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD607,
                            (uint16_t[]){
                                0xa1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD608,
                            (uint16_t[]){
                                0xc0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD609,
                            (uint16_t[]){
                                0xee,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD60A,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD60B,
                            (uint16_t[]){
                                0x10,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD60C,
                            (uint16_t[]){
                                0x2c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD60D,
                            (uint16_t[]){
                                0x43,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD60E,
                            (uint16_t[]){
                                0x57,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD60F,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD610,
                            (uint16_t[]){
                                0x68,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD611,
                            (uint16_t[]){
                                0x78,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD612,
                            (uint16_t[]){
                                0x87,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD613,
                            (uint16_t[]){
                                0x94,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD614,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD615,
                            (uint16_t[]){
                                0xa0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD616,
                            (uint16_t[]){
                                0xac,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD617,
                            (uint16_t[]){
                                0xb6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD618,
                            (uint16_t[]){
                                0xc1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD619,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD61A,
                            (uint16_t[]){
                                0xcb,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD61B,
                            (uint16_t[]){
                                0xcd,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD61C,
                            (uint16_t[]){
                                0xd6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD61D,
                            (uint16_t[]){
                                0xdf,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD61E,
                            (uint16_t[]){
                                0x95,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD61F,
                            (uint16_t[]){
                                0xe8,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD620,
                            (uint16_t[]){
                                0xf1,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD621,
                            (uint16_t[]){
                                0xfa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD622,
                            (uint16_t[]){
                                0x02,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD623,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD624,
                            (uint16_t[]){
                                0x0b,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD625,
                            (uint16_t[]){
                                0x13,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD626,
                            (uint16_t[]){
                                0x1d,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD627,
                            (uint16_t[]){
                                0x26,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD628,
                            (uint16_t[]){
                                0xaa,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD629,
                            (uint16_t[]){
                                0x30,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD62A,
                            (uint16_t[]){
                                0x3c,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD62B,
                            (uint16_t[]){
                                0x4A,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD62C,
                            (uint16_t[]){
                                0x63,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD62D,
                            (uint16_t[]){
                                0xea,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD62E,
                            (uint16_t[]){
                                0x79,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD62F,
                            (uint16_t[]){
                                0xa6,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD630,
                            (uint16_t[]){
                                0xd0,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD631,
                            (uint16_t[]){
                                0x20,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD632,
                            (uint16_t[]){
                                0x0f,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD633,
                            (uint16_t[]){
                                0x8e,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xD634,
                            (uint16_t[]){
                                0xff,
                            },
                            2);

  // AVDD VOLTAGE SETTING
  esp_lcd_panel_io_tx_param(io, 0xB000,
                            (uint16_t[]){
                                0x05,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB001,
                            (uint16_t[]){
                                0x05,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB002,
                            (uint16_t[]){
                                0x05,
                            },
                            2);
  // AVEE VOLTAGE SETTING
  esp_lcd_panel_io_tx_param(io, 0xB100,
                            (uint16_t[]){
                                0x05,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB101,
                            (uint16_t[]){
                                0x05,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB102,
                            (uint16_t[]){
                                0x05,
                            },
                            2);
  // AVDD Boosting
  esp_lcd_panel_io_tx_param(io, 0xB600,
                            (uint16_t[]){
                                0x34,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB601,
                            (uint16_t[]){
                                0x34,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB603,
                            (uint16_t[]){
                                0x34,
                            },
                            2);
  // AVEE Boosting
  esp_lcd_panel_io_tx_param(io, 0xB700,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB701,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB702,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  // VCL Boosting
  esp_lcd_panel_io_tx_param(io, 0xB800,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB801,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB802,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  // VGLX VOLTAGE SETTING
  esp_lcd_panel_io_tx_param(io, 0xBA00,
                            (uint16_t[]){
                                0x14,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xBA01,
                            (uint16_t[]){
                                0x14,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xBA02,
                            (uint16_t[]){
                                0x14,
                            },
                            2);
  // VCL Boosting
  esp_lcd_panel_io_tx_param(io, 0xB900,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB901,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xB902,
                            (uint16_t[]){
                                0x24,
                            },
                            2);
  // Gamma Voltage
  esp_lcd_panel_io_tx_param(io, 0xBc00,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xBc01,
                            (uint16_t[]){
                                0xa0,
                            },
                            2);  // vgmp=5.0
  esp_lcd_panel_io_tx_param(io, 0xBc02,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xBd00,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xBd01,
                            (uint16_t[]){
                                0xa0,
                            },
                            2);  // vgmn=5.0
  esp_lcd_panel_io_tx_param(io, 0xBd02,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  // VCOM Setting
  esp_lcd_panel_io_tx_param(io, 0xBe01,
                            (uint16_t[]){
                                0x3d,
                            },
                            2);  // 3
  // ENABLE PAGE 0
  esp_lcd_panel_io_tx_param(io, 0xF000,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF001,
                            (uint16_t[]){
                                0xAA,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF002,
                            (uint16_t[]){
                                0x52,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF003,
                            (uint16_t[]){
                                0x08,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF004,
                            (uint16_t[]){
                                0x00,
                            },
                            2);
  // Vivid Color Function Control
  esp_lcd_panel_io_tx_param(io, 0xB400,
                            (uint16_t[]){
                                0x10,
                            },
                            2);
  // Z-INVERSION
  esp_lcd_panel_io_tx_param(io, 0xBC00,
                            (uint16_t[]){
                                0x05,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xBC01,
                            (uint16_t[]){
                                0x05,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xBC02,
                            (uint16_t[]){
                                0x05,
                            },
                            2);

  //*************** add on 20111021**********************//
  esp_lcd_panel_io_tx_param(io, 0xB700,
                            (uint16_t[]){
                                0x22,
                            },
                            2);  // GATE EQ CONTROL
  esp_lcd_panel_io_tx_param(io, 0xB701,
                            (uint16_t[]){
                                0x22,
                            },
                            2);  // GATE EQ CONTROL
  esp_lcd_panel_io_tx_param(io, 0xC80B,
                            (uint16_t[]){
                                0x2A,
                            },
                            2);  // DISPLAY TIMING CONTROL
  esp_lcd_panel_io_tx_param(io, 0xC80C,
                            (uint16_t[]){
                                0x2A,
                            },
                            2);  // DISPLAY TIMING CONTROL
  esp_lcd_panel_io_tx_param(io, 0xC80F,
                            (uint16_t[]){
                                0x2A,
                            },
                            2);  // DISPLAY TIMING CONTROL
  esp_lcd_panel_io_tx_param(io, 0xC810,
                            (uint16_t[]){
                                0x2A,
                            },
                            2);  // DISPLAY TIMING CONTROL
  //*************** add on 20111021**********************//
  // PWM_ENH_OE =1
  esp_lcd_panel_io_tx_param(io, 0xd000,
                            (uint16_t[]){
                                0x01,
                            },
                            2);
  // DM_SEL =1
  esp_lcd_panel_io_tx_param(io, 0xb300,
                            (uint16_t[]){
                                0x10,
                            },
                            2);
  // VBPDA=07h
  esp_lcd_panel_io_tx_param(io, 0xBd02,
                            (uint16_t[]){
                                0x07,
                            },
                            2);
  // VBPDb=07h
  esp_lcd_panel_io_tx_param(io, 0xBe02,
                            (uint16_t[]){
                                0x07,
                            },
                            2);
  // VBPDc=07h
  esp_lcd_panel_io_tx_param(io, 0xBf02,
                            (uint16_t[]){
                                0x07,
                            },
                            2);
  // ENABLE PAGE 2
  esp_lcd_panel_io_tx_param(io, 0xF000,
                            (uint16_t[]){
                                0x55,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF001,
                            (uint16_t[]){
                                0xAA,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF002,
                            (uint16_t[]){
                                0x52,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF003,
                            (uint16_t[]){
                                0x08,
                            },
                            2);
  esp_lcd_panel_io_tx_param(io, 0xF004,
                            (uint16_t[]){
                                0x02,
                            },
                            2);
  // SDREG0 =0
  esp_lcd_panel_io_tx_param(io, 0xc301,
                            (uint16_t[]){
                                0xa9,
                            },
                            2);
  // DS=14
  esp_lcd_panel_io_tx_param(io, 0xfe01,
                            (uint16_t[]){
                                0x94,
                            },
                            2);
  // OSC =60h
  esp_lcd_panel_io_tx_param(io, 0xf600,
                            (uint16_t[]){
                                0x60,
                            },
                            2);
  // TE ON
  esp_lcd_panel_io_tx_param(io, 0x3500,
                            (uint16_t[]){
                                0x00,
                            },
                            2);

  // //SLEEP OUT
  // esp_lcd_panel_io_tx_param(io, 0x1100, NULL, 0);
  // vTaskDelay(100 / portTICK_PERIOD_MS);

  // //DISPLY ON
  // esp_lcd_panel_io_tx_param(io, 0x2900, NULL, 0);
  // vTaskDelay(100 / portTICK_PERIOD_MS);

  esp_lcd_panel_io_tx_param(io, 0x3A00,
                            (uint16_t[]){
                                0x55,
                            },
                            2);  // COLMOD
  esp_lcd_panel_io_tx_param(io, 0x3600,
                            (uint16_t[]){
                                0xA3,
                            },
                            2);  // Memory Data Access Control
}
