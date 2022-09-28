//Pins definition for ESP32 ESP32S3_HMI43V3
//I2C bus
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32-S3-HMI-DevKit I2C GPIO definition
 *
 */
#define FUNC_I2C_EN     (1)
#define I2C_SCL_PIN    (39) //GPIO 39
#define I2C_SDA_PIN    (40) //GPIO 40
#define I2C_CLK_SPEED  (400000)
#define I2C_PORT_NUMBER (0)

#ifdef __cplusplus
} /* extern "C" */
#endif
