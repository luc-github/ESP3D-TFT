#ifndef _KEY_H_
#define _KEY_H_

#include "esp_err.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "bsp_board.h"


esp_err_t key_init_isr(void);


#endif

