/*
  esp3d-tft project

  Copyright (c) 2022 Luc Lebosse. All rights reserved.

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

/*********************
 *      INCLUDES
 *********************/
#include "esp_log.h"
#define LOG_TAG "BSP"
#include "lvgl.h"
#include "bsp.h"
#include "xpt2046.h"
#include "ili9341.h"
#include "spi_bus.h"
#include "touch_spi.h"
#include "disp_spi.h"
#include "esp_lcd_backlight.h"



/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void bsp_init(void)
{
    //Driver initialization
    ESP_LOGI(LOG_TAG, "Display buffer size: %d", DISP_BUF_SIZE);

    /* Display controller initialization */
    ESP_LOGI(LOG_TAG, "Initializing SPI master for display");

    spi_driver_init(DISP_SPI_HOST,
                    DISP_SPI_MISO, DISP_SPI_MOSI, DISP_SPI_CLK,
                    DISP_SPI_BUS_MAX_TRANSFER_SZ, 1,
                    DISP_SPI_IO2, DISP_SPI_IO3);

    ESP_LOGI(LOG_TAG, "Initializing display controller");
    disp_spi_add_device(DISP_SPI_HOST);

    ili9341_init();
#if (defined(DISP_BACKLIGHT_SWITCH) || defined(DISP_BACKLIGHT_PWM))
    const disp_backlight_config_t bckl_config = {
        .gpio_num = DISP_PIN_BCKL,
#if defined DISP_BACKLIGHT_PWM
        .pwm_control = true,
#else
        .pwm_control = false,
#endif
#if defined BACKLIGHT_ACTIVE_LVL
        .output_invert = false, // Backlight on high
#else
        .output_invert = true, // Backlight on low
#endif
        .timer_idx = 0,
        .channel_idx = 0 // @todo this prevents us from having two PWM controlled displays
    };
    disp_backlight_h bckl_handle = disp_backlight_new(&bckl_config);
    disp_backlight_set(bckl_handle, 100);
#endif
    /* Touch controller initialization */
    ESP_LOGI(LOG_TAG, "Initializing SPI master for touch");
    spi_driver_init(TOUCH_SPI_HOST,
                    TOUCH_SPI_MISO, TOUCH_SPI_MOSI, TOUCH_SPI_CLK,
                    0 /* Defaults to 4094 */, 2,
                    -1, -1);
                    
    ESP_LOGI(LOG_TAG, "Initializing touch controller");
    tp_spi_add_device(TOUCH_SPI_HOST);
    xpt2046_init();

    //Lvgl initialization
    lv_init();

    //Lvgl setup
    ESP_LOGI(LOG_TAG, "Setup Lvgl");
    lv_color_t* buf1 = (lv_color_t*)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), HAS_PSRAM ?MALLOC_CAP_SPIRAM: MALLOC_CAP_DMA);
    if (buf1 == NULL) return ESP_FAIL;

    /* Use double buffered when not working with monochrome displays */
    lv_color_t* buf2 = (lv_color_t*)heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t),  HAS_PSRAM ?MALLOC_CAP_SPIRAM: MALLOC_CAP_DMA);
    if (buf2 == NULL) return ESP_FAIL;


    static lv_disp_draw_buf_t draw_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    /* Initialize the working buffer depending on the selected display.*/
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, size_in_px);

    static lv_disp_drv_t disp_drv;        /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
    disp_drv.flush_cb = ili9341_flush;    /*Set your driver function*/
    disp_drv.draw_buf = &draw_buf;        /*Assign the buffer to the display*/
    disp_drv.hor_res = DISP_HOR_RES_MAX;   /*Set the horizontal resolution of the display*/
    disp_drv.ver_res = DISP_VER_RES_MAX;   /*Set the vertical resolution of the display*/
    lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/



    /* Register an input device */
    static lv_indev_drv_t indev_drv;           /*Descriptor of a input device driver*/
    lv_indev_drv_init(&indev_drv);             /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = xpt2046_read;          /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

    return ESP_OK;
}

