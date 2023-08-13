//Pins definition for ESP32 ESP32S3_BZM_TFT35_GT911
//I2C bus
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ESP32-S3-HMI-DevKit I2C GPIO definition
 *
 */
#define I2C_SCL_PIN    (39) //GPIO 5
#define I2C_SDA_PIN    (40) //GPIO 6
#define I2C_CLK_SPEED  (400000)
#define I2C_PORT_NUMBER (0)

#ifdef __cplusplus
} /* extern "C" */
#endif
