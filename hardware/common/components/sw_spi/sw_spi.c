/**
 * @file sw_spi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "sw_spi.h"
#include "esp3d_log.h"
#include "esp_system.h"
#include <driver/gpio.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void sw_spi_acquire_bus();
static void sw_spi_release_bus();
static void sw_spi_write_byte(uint8_t data);
static uint8_t sw_spi_read_byte();

/**********************
 *  STATIC VARIABLES
 **********************/
static const sw_spi_config_t *_config = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void sw_spi_init(const sw_spi_config_t *config) {
  assert(config != NULL);
  _config = config;

  esp3d_log("Initializing software SPI pins...");
  esp3d_log("MISO pin: %d, MOSI pin: %d, SCLK pin: %d, CS pin: %d",
        config->miso_pin, config->mosi_pin, config->clk_pin, config->cs_pin);

	esp_rom_gpio_pad_select_gpio(config->cs_pin);
  gpio_set_direction(config->cs_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(config->cs_pin, 1);

	esp_rom_gpio_pad_select_gpio(config->clk_pin);
  gpio_set_direction(config->clk_pin, GPIO_MODE_OUTPUT);

  esp_rom_gpio_pad_select_gpio(config->mosi_pin);
  gpio_set_direction(config->mosi_pin, GPIO_MODE_OUTPUT);

  gpio_config_t miso_config = {
      .pin_bit_mask = BIT64(config->miso_pin),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  ESP_ERROR_CHECK(gpio_config(&miso_config));
}

uint16_t sw_spi_read_reg16(uint8_t reg) {
	sw_spi_acquire_bus();
	sw_spi_write_byte(reg);  
	uint8_t d0 = sw_spi_read_byte();
  uint8_t d1 = sw_spi_read_byte();
	sw_spi_release_bus();
	return ((uint16_t)d0 << 8) | d1;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void sw_spi_acquire_bus() {
  gpio_set_level(_config->clk_pin, 0);
  gpio_set_level(_config->mosi_pin, 0);
  gpio_set_level(_config->cs_pin, 0);
}

static void sw_spi_release_bus() {
	gpio_set_level(_config->cs_pin, 1);
}

static void sw_spi_write_byte(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    // set MOSI
		gpio_set_level(_config->mosi_pin, (data & 0x80) ? 1 : 0);
    gpio_set_level(_config->clk_pin, 1);
		data <<= 1;
		// (read MISO)
		// if (gpio_get_level(_config->miso_pin)) {
    //   data++;
    // }
    gpio_set_level(_config->clk_pin, 0);
  }
}

static uint8_t sw_spi_read_byte() {
	uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++) {
		// (set MOSI)
		// gpio_set_level(_config->mosi_pin, (data & 0x80) ? 1 : 0);
		gpio_set_level(_config->clk_pin, 1);
		data <<= 1;
		// read MISO
		if (gpio_get_level(_config->miso_pin)) {
      data++;
    }
    gpio_set_level(_config->clk_pin, 0);
  }
	return data;
}
