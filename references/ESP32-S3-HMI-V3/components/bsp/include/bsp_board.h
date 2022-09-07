
#ifndef _BSP_BOARD_H_
#define _BSP_BOARD_H_

#include "driver/gpio.h"

/**
 * @brief ESP32-S3-HMI-DevKit LCD GPIO defination and config
 * 
 */
#define FUNC_LCD_EN     (1)
#define LCD_BIT_WIDTH   (16)

#define GPIO_LCD_BL     (GPIO_NUM_NC)
#define GPIO_LCD_CS     (GPIO_NUM_NC)
#define GPIO_LCD_RST    (GPIO_NUM_21)
#define GPIO_LCD_RS     (GPIO_NUM_38)
#define GPIO_LCD_WR     (GPIO_NUM_17)

#define GPIO_LCD_D00    (GPIO_NUM_1)
#define GPIO_LCD_D01    (GPIO_NUM_9)
#define GPIO_LCD_D02    (GPIO_NUM_2)
#define GPIO_LCD_D03    (GPIO_NUM_10)
#define GPIO_LCD_D04    (GPIO_NUM_3)
#define GPIO_LCD_D05    (GPIO_NUM_11)
#define GPIO_LCD_D06    (GPIO_NUM_4)
#define GPIO_LCD_D07    (GPIO_NUM_12)
#define GPIO_LCD_D08    (GPIO_NUM_5)
#define GPIO_LCD_D09    (GPIO_NUM_13)
#define GPIO_LCD_D10    (GPIO_NUM_6)
#define GPIO_LCD_D11    (GPIO_NUM_14)
#define GPIO_LCD_D12    (GPIO_NUM_7)
#define GPIO_LCD_D13    (GPIO_NUM_15)
#define GPIO_LCD_D14    (GPIO_NUM_8)
#define GPIO_LCD_D15    (GPIO_NUM_16)

#define LCD_DIRECTION_LANDSCAPE   (1)  // 1:landscape mode   0:portrait mode

#if LCD_DIRECTION_LANDSCAPE   // landscape mode
#define LCD_WIDTH       (800)
#define LCD_HEIGHT      (480)
#else   // portrait mode
#define LCD_WIDTH       (480)
#define LCD_HEIGHT      (800)
#endif

/**
 * @brief ESP32-S3-HMI-DevKit I2C GPIO defineation
 * 
 */
#define FUNC_I2C_EN     (1)
#define GPIO_I2C_SCL    (GPIO_NUM_39)
#define GPIO_I2C_SDA    (GPIO_NUM_40)

/**
 * @brief ESP32-S3-HMI-DevKit SPI GPIO defination
 * 
 */
#define FUNC_SPI_EN     (1)
#define GPIO_SPI_CS_SD  (GPIO_NUM_34)
#define GPIO_SPI_MISO   (GPIO_NUM_37)
#define GPIO_SPI_MOSI   (GPIO_NUM_35)
#define GPIO_SPI_SCLK   (GPIO_NUM_36)

/**
 * @brief ESP32-S3-HMI-DevKit KEY GPIO defination
 * 
 */
#define FUNC_KEY_EN     (1)
#define GPIO_KEY        (GPIO_NUM_0)

/**
 * @brief ESP32-S3-HMI-DevKit ADC GPIO defination
 * 
 */
#define FUNC_ADC_EN     (1)
#define GPIO_BAT_ADC    (GPIO_NUM_18)

/**
 * @brief ESP32-S3-HMI-DevKit I2S GPIO defination
 * 
 */
#define FUNC_I2S_EN      (1)
#define GPIO_I2S_MCLK    (GPIO_NUM_45)
#define GPIO_I2S_SCLK    (GPIO_NUM_42)
#define GPIO_I2S_LRCK    (GPIO_NUM_48)
#define GPIO_I2S_DIN     (GPIO_NUM_41)
#define GPIO_I2S_DOUT    (GPIO_NUM_47)

#endif /* CONFIG_ESP32_S3_HMI_DEVKIT_BOARD */

