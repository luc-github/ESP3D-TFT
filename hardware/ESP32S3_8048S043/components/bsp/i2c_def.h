// Pins definition for ESP32S3_8048S050
// I2C bus
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32-S3-HMI-DevKit I2C GPIO definition
 *
 */
#define I2C_SCL_PIN (20)  // GPIO 20
#define I2C_SDA_PIN (19)  // GPIO 19
#define I2C_CLK_SPEED (400000)
#define I2C_PORT_NUMBER (0)

#ifdef __cplusplus
} /* extern "C" */
#endif
