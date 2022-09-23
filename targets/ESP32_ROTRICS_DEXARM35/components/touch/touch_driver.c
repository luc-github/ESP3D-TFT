/**
 * @file touch_driver.c
 */

#include "touch_driver.h"
#include "tp_spi.h"


void touch_driver_init(void)
{
    xpt2046_init();
}


void touch_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data)

{
    bool res = false;
    res = xpt2046_read(drv, data);
    data->continue_reading = res;

}

