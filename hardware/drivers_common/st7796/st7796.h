/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 * Modified by Luc LEBOSSE 2024
 *
 * SPDX-License-Identifier: Apache-2.0
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
 * GLOBAL PROTOTYPES
 **********************/
/**
 * @brief Creates a new ST7796 SPI LCD panel.
 *
 * This function creates a new ST7796 LCD panel using the provided I/O handle, panel device configuration, and returns a handle to the created panel.
 *
 * @param io The I/O handle for the LCD panel.
 * @param panel_dev_config The device configuration for the LCD panel.
 * @param ret_panel Pointer to the variable where the handle to the created panel will be stored.
 * @return `ESP_OK` if the panel is created successfully, or an error code if an error occurred.
 */
esp_err_t esp_lcd_new_panel_st7796(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);


#ifdef __cplusplus
} /* extern "C" */
#endif
