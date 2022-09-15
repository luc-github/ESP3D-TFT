/**
 * @file disp_driver.c
 */

#include "disp_driver.h"
#include "disp_spi.h"
#include "esp_lcd_backlight.h"

void *disp_driver_init(void)
{
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
    return bckl_handle;
#else
    return NULL;
#endif
}

void disp_driver_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
    ili9341_flush(drv, area, color_map);
}

