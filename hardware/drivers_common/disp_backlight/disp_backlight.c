/*
  disp_backlight.c

  Copyright (c) 2023 Luc Lebosse. All rights reserved.

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
#include "disp_backlight.h"

#include <driver/gpio.h>
#include <driver/ledc.h>
#include <soc/gpio_sig_map.h>
#include <soc/ledc_periph.h>  // to invert LEDC output on IDF version < v4.3

#include "esp3d_log.h"

/**
 * @brief Creates a display backlight instance.
 *
 * This function creates a display backlight instance based on the provided
 * configuration.
 *
 * @param config Pointer to the backlight configuration.
 * @param bckl Pointer to the created backlight instance.
 * @return `ESP_OK` if the backlight instance is created successfully, or an
 * error code if it fails.
 */
esp_err_t disp_backlight_create(const disp_backlight_config_t *config,
                                disp_backlight_t **bckl) {
  // Check input parameters
  if (config == NULL) {
    esp3d_log_e("Invalid backlight configuration");
    return ESP_ERR_INVALID_ARG;
  }
  if (!GPIO_IS_VALID_OUTPUT_GPIO(config->gpio_num)) {
    esp3d_log_e("Invalid GPIO number");
    return ESP_ERR_INVALID_ARG;
  }
  *bckl = (disp_backlight_t *)calloc(1, sizeof(disp_backlight_t));
  if (*bckl == NULL) {
    esp3d_log_e("Not enough memory");
    return ESP_ERR_NO_MEM;
  }

  if (config->pwm_control) {
    // Configure LED (Backlight) pin as PWM for Brightness control.
    esp3d_log("Configuring backlight with PWM control");
    (*bckl)->pwm_control = true;
    (*bckl)->index = config->channel_idx;
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = config->gpio_num,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = config->channel_idx,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = config->timer_idx,
        .duty = 0,
        .hpoint = 0};
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = config->timer_idx,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK};

    if (ledc_timer_config(&LCD_backlight_timer) != ESP_OK) {
      esp3d_log_e("Failed to configure LEDC timer");
      free(*bckl);
      *bckl = NULL;
      return ESP_ERR_INVALID_ARG;
    }
    if (ledc_channel_config(&LCD_backlight_channel) != ESP_OK) {
      esp3d_log_e("Failed to configure LEDC channel");
      free(*bckl);
      *bckl = NULL;
      return ESP_ERR_INVALID_ARG;
    }
    esp_rom_gpio_connect_out_signal(
        config->gpio_num,
        ledc_periph_signal[LEDC_LOW_SPEED_MODE].sig_out0_idx +
            config->channel_idx,
        config->output_invert, 0);
  } else {
    // Configure GPIO for output
    esp3d_log("Configuring backlight with GPIO control");
    (*bckl)->index = config->gpio_num;
    esp_rom_gpio_pad_select_gpio(config->gpio_num);
    if (gpio_set_direction(config->gpio_num, GPIO_MODE_OUTPUT) != ESP_OK) {
      esp3d_log_e("Failed to set GPIO direction");
      free(*bckl);
      return ESP_ERR_INVALID_ARG;
    }
    esp_rom_gpio_connect_out_signal(config->gpio_num, SIG_GPIO_OUT_IDX,
                                    config->output_invert, false);

    esp3d_log("Backlight created successfully");
  }

  return ESP_OK;
}

/**
 * @brief Sets the brightness of the display backlight.
 *
 * This function sets the brightness of the display backlight to the specified
 * percentage.
 *
 * @param bckl Pointer to the `disp_backlight_t` structure representing the
 * display backlight.
 * @param brightness_percent The brightness level to set, specified as a
 * percentage (0-100).
 *
 * @return `ESP_OK` if the brightness was set successfully, or an error code if
 * an error occurred.
 */
esp_err_t disp_backlight_set(disp_backlight_t *bckl, int brightness_percent) {
  // Check input paramters
  if (bckl == NULL) {
    esp3d_log("Invalid backlight handle");
    return ESP_ERR_INVALID_ARG;
  }
  // Do some sanity checks
  if (brightness_percent > 100) {
    brightness_percent = 100;
  }
  if (brightness_percent < 0) {
    brightness_percent = 0;
  }

  esp3d_log("Setting LCD backlight: %d%%", brightness_percent);
  // Apply the brightness
  if (bckl->pwm_control) {
    uint32_t duty_cycle =
        (1023 * brightness_percent) /
        100;  // LEDC resolution set to 10bits, thus: 100% = 1023
    if (ledc_set_duty(LEDC_LOW_SPEED_MODE, bckl->index, duty_cycle) != ESP_OK) {
      esp3d_log_e("Failed to set LEDC duty cycle");
      return ESP_ERR_INVALID_ARG;
    }
    if (ledc_update_duty(LEDC_LOW_SPEED_MODE, bckl->index) != ESP_OK) {
      esp3d_log_e("Failed to update LEDC duty cycle");
      return ESP_ERR_INVALID_ARG;
    }
  } else {
    if (gpio_set_level(bckl->index, brightness_percent) != ESP_OK) {
      esp3d_log_e("Failed to set GPIO level");
      return ESP_ERR_INVALID_ARG;
    }
  }
  esp3d_log("LCD backlight set successfully");
  return ESP_OK;
}

/**
 * @brief Deletes a disp_backlight_t object and frees the associated memory.
 *
 * This function deletes a disp_backlight_t object and frees the memory
 * allocated for it.
 *
 * @param bckl Pointer to the pointer of the disp_backlight_t object to be
 * deleted. After successful deletion, the pointer will be set to NULL.
 *
 * @return
 *     - ESP_OK if the disp_backlight_t object was successfully deleted.
 *     - ESP_ERR_INVALID_ARG if the pointer to the disp_backlight_t object is
 * NULL.
 */
esp_err_t disp_backlight_delete(disp_backlight_t **bckl) {
  esp_err_t err = ESP_OK;
  if (bckl == NULL || *bckl == NULL) {
    esp3d_log_e("Invalid backlight handle");
    return ESP_ERR_INVALID_ARG;
  }

  if ((*bckl)->pwm_control) {
    err = ledc_stop(LEDC_LOW_SPEED_MODE, (*bckl)->index, 0);
    if (err != ESP_OK) {
      esp3d_log_e("Failed to stop LEDC");
    } else {
      esp3d_log("PWM control stopped");
    }
  } else {
    if (GPIO_IS_VALID_GPIO((*bckl)->index)) {
      gpio_reset_pin((*bckl)->index);
      esp3d_log("GPIO pin reset");
    } else {
      esp3d_log_e("Invalid GPIO pin");
      err = ESP_ERR_INVALID_ARG;
    }
  }

  free(*bckl);
  *bckl = NULL;
  if (err == ESP_OK) {
    esp3d_log("Backlight deleted successfully");
  } else {
    esp3d_log_e("Failed to delete backlight");
  }
  return err;
}
