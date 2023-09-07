/**
 * @file XPT2046.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "xpt2046.h"
#include <stddef.h>
#include "esp_system.h"
#include "esp3d_log.h"
#include <driver/gpio.h>

/*********************
 *      DEFINES
 *********************/

#define CMD_X_READ  0b11010000
#define CMD_Y_READ  0b10010000
#define CMD_Z1_READ 0b10110000
#define CMD_Z2_READ 0b11000000

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    TOUCH_NOT_DETECTED = 0,
    TOUCH_DETECTED = 1,
} xpt2046_touch_detect_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static xpt2046_touch_detect_t xpt2048_is_touch_detected();

/**********************
 *  STATIC VARIABLES
 **********************/
static const xpt2046_config_t *xpt2046_config = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_err_t xpt2046_init(const xpt2046_config_t *config) {
    if (config == NULL || config->read_reg12_fn == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    xpt2046_config = config;

    esp3d_log("Init XPT2046");
    if (GPIO_IS_VALID_GPIO(config->irq_pin)) {
        gpio_config_t irq_config = {
            .pin_bit_mask = BIT64(config->irq_pin),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };    
        ESP_ERROR_CHECK(gpio_config(&irq_config));        
    }

    return ESP_OK;
}

xpt2046_data_t xpt2046_read() {
    xpt2046_data_t data = {.is_pressed = false, .x = -1, .y = -1};
    if (xpt2046_config == NULL) {
        return data;
    }
    if (xpt2048_is_touch_detected() == TOUCH_DETECTED) {
        data.is_pressed = true;
        data.x = xpt2046_config->read_reg12_fn(CMD_X_READ);
        data.y = xpt2046_config->read_reg12_fn(CMD_Y_READ);
        if (xpt2046_config->swap_xy) {
            int16_t swap_tmp = data.x;
            data.x = data.y;
            data.y = swap_tmp;
        }
        if (xpt2046_config->invert_x) {
            data.x = 4095 - data.x;
        }
        if (xpt2046_config->invert_y) {
            data.y = 4095 - data.y;
        }
        esp3d_log("P(%d,%d)", data.x, data.y);
    }
    return data;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static xpt2046_touch_detect_t xpt2048_is_touch_detected() {
    if (xpt2046_config == NULL) {
        return TOUCH_NOT_DETECTED;
    }

    // Check if IRQ pin is triggered
    if (xpt2046_config->irq_pin >= 0) {
        uint8_t irq = gpio_get_level(xpt2046_config->irq_pin);
        if (irq != 0) {
            return TOUCH_NOT_DETECTED;
        }
    }

    // Check touch pressure
    int16_t z1 = xpt2046_config->read_reg12_fn(CMD_Z1_READ);
    int16_t z2 = xpt2046_config->read_reg12_fn(CMD_Z2_READ);

    // This is not what the confusing datasheet says but it seems to
    //   be enough to detect real touches on the panel.
    int16_t z = z1 + 4095 - z2;
    if (z < xpt2046_config->touch_threshold) {
        return TOUCH_NOT_DETECTED;
    }

    return TOUCH_DETECTED;
}
