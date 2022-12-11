/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <stdlib.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp3d_log.h"
#include "esp_check.h"
#include "disp_def.h"
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
    uint8_t colmod_cal; // save surrent value of LCD_CMD_COLMOD register
} st7796_panel_t;


/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool st7796_notify_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
esp_err_t esp_lcd_new_panel_st7796(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);
esp_lcd_panel_handle_t panel_handle = NULL;

static esp_err_t panel_st7796_del(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_init(esp_lcd_panel_t *panel);
static esp_err_t panel_st7796_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_st7796_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_st7796_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_st7796_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_st7796_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_st7796_disp_off(esp_lcd_panel_t *panel, bool off);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_lcd_panel_handle_t * st7796_panel_handle(){
    return &panel_handle;
} 

static bool st7796_notify_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

void st7796_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) disp_drv->user_data;

    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
}

esp_err_t st7796_init(lv_disp_drv_t  * disp_drv){

    if (panel_handle!= NULL) {
        esp3d_log("st7796 bus already initialized");
        return ESP_FAIL;
    }
    esp3d_log("Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << DISP_BL_PIN
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_set_level(DISP_BL_PIN, DISP_BL_OFF);

    esp3d_log("init st7796 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = DISP_RS_PIN,
        .wr_gpio_num = DISP_WR_PIN,
        .data_gpio_nums = {
            DISP_D00_PIN, DISP_D01_PIN, DISP_D02_PIN, DISP_D03_PIN, 
            DISP_D04_PIN, DISP_D05_PIN, DISP_D06_PIN, DISP_D07_PIN
        },
        .bus_width = DISP_BITS_WIDTH,
        .max_transfer_bytes = DISP_HOR_RES_MAX * 40 * sizeof(uint16_t),  
        .psram_trans_align = 64, // could be 16 32 64
        .sram_trans_align = 4 // no idea of the alignment use sample one
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));
    if (i80_bus==NULL){
        esp3d_log("init lcd i80 display bus failed");
    }

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = DISP_CS_PIN,
        .pclk_hz = DISP_CLK_FREQ,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .on_color_trans_done = st7796_notify_flush_ready,        //Callback invoked when color data transfer has finished
        .user_ctx = disp_drv,                    //User private data, passed directly to on_color_trans_done’s user_ctx
        .lcd_cmd_bits = DISP_CMD_BITS_WIDTH,
        .lcd_param_bits = DISP_PARAM_BITS_WIDTH,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = DISP_RST_PIN,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);   // LCD Reset
    esp_lcd_panel_init(panel_handle);    // LCD init

    esp_lcd_panel_invert_color(panel_handle, true);
    #if DISP_DIRECTION_LANDSCAPE == 1  // landscape mode
    panel_st7796_swap_xy(panel_handle,true);
    #endif //DISP_DIRECTION_LANDSCAPE
    esp3d_log("Turn on LCD backlight");
    gpio_set_level(DISP_BL_PIN, DISP_BL_ON);

    return ESP_OK;
}


/**********************
 *   Static FUNCTIONS
 **********************/

esp_err_t esp_lcd_new_panel_st7796(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    st7796_panel_t *st7796 = NULL;
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, "", "invalid argument");
    st7796 = calloc(1, sizeof(st7796_panel_t));
    ESP_GOTO_ON_FALSE(st7796, ESP_ERR_NO_MEM, err, "", "no mem for st7796 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, "", "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->color_space) {
    case ESP_LCD_COLOR_SPACE_RGB:
        st7796->madctl_val = 0;
        break;
    case ESP_LCD_COLOR_SPACE_BGR:
        st7796->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "", "unsupported color space");
        break;
    }

    switch (panel_dev_config->bits_per_pixel) {
    case 16:
        st7796->colmod_cal = 0x55;
        break;
    case 18:
        st7796->colmod_cal = 0x66;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, "", "unsupported pixel width");
        break;
    }

    st7796->io = io;
    st7796->bits_per_pixel = panel_dev_config->bits_per_pixel;
    st7796->reset_gpio_num = panel_dev_config->reset_gpio_num;
    st7796->reset_level = panel_dev_config->flags.reset_active_high;
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
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(st7796);
    }
    return ret;
}

static esp_err_t panel_st7796_del(esp_lcd_panel_t *panel)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);

    if (st7796->reset_gpio_num >= 0) {
        gpio_reset_pin(st7796->reset_gpio_num);
    }
    esp3d_log("del st7796 panel @%p", st7796);
    free(st7796);
    return ESP_OK;
}

static esp_err_t panel_st7796_reset(esp_lcd_panel_t *panel)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;

    // perform hardware reset
    if (st7796->reset_gpio_num >= 0) {
        gpio_set_level(st7796->reset_gpio_num, st7796->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(st7796->reset_gpio_num, !st7796->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    } else { // perform software reset
        esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(20)); // spec, wait at least 5m before sending new command
    }

    return ESP_OK;
}

static esp_err_t panel_st7796_init(esp_lcd_panel_t *panel)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;
    // LCD goes into sleep mode and display will be turned off after power on reset, exit sleep mode first
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        st7796->madctl_val,
    }, 1);
    esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, (uint8_t[]) {
        st7796->colmod_cal,
    }, 1);
    // turn on display
    esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);

    return ESP_OK;
}

static esp_err_t panel_st7796_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
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

static esp_err_t panel_st7796_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
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

static esp_err_t panel_st7796_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
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
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        st7796->madctl_val
    }, 1);
    return ESP_OK;
}

static esp_err_t panel_st7796_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;
    if (swap_axes) {
        st7796->madctl_val |= LCD_CMD_MV_BIT;
    } else {
        st7796->madctl_val &= ~LCD_CMD_MV_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        st7796->madctl_val
    }, 1);
    return ESP_OK;
}

static esp_err_t panel_st7796_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    st7796->x_gap = x_gap;
    st7796->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_st7796_disp_off(esp_lcd_panel_t *panel, bool off)
{
    st7796_panel_t *st7796 = __containerof(panel, st7796_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7796->io;
    int command = 0;
    if (off) {
        command = LCD_CMD_DISPOFF;
    } else {
        command = LCD_CMD_DISPON;
    }
    esp_lcd_panel_io_tx_param(io, command, NULL, 0);
    return ESP_OK;
}
