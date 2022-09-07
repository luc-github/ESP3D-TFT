#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_log.h"

#include "driver/adc_common.h"
#include "esp_adc_cal.h"
#include "esp_event.h"
#include "bsp_board.h"


#ifdef __cplusplus
extern "C"
{
#endif


esp_err_t adc_init(void);

uint16_t adc_get_voltage(void);


#ifdef __cplusplus
}
#endif