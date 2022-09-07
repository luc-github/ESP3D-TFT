#ifndef _RM68120_H_
#define _RM68120_H_

#include <stdlib.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "bsp_lcd.h"

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    uint16_t width;          // Current screen width, it may change when apply to rotate
    uint16_t height;         // Current screen height, it may change when apply to rotate
    uint8_t dir;             // Current screen direction
    int x_gap;
    int y_gap;
    unsigned int bits_per_pixel;
    uint8_t madctl_val;     // MADCTL register addr:3600H
    uint8_t colmod_cal;     // Color Format: COLMOD register addr:3A00H
} lcd_panel_t;


esp_err_t esp_lcd_new_panel_rm68120(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);


#endif

