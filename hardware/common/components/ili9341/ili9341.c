/**
 * @file ili9341.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ili9341.h"
#include "disp_spi.h"
#include <driver/gpio.h>
#include "esp3d_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void ili9341_send_cmd(uint8_t cmd);
static void ili9341_send_data(void * data, uint16_t length);
static void ili9341_send_color(void * data, uint16_t length);

/**********************
 *  STATIC VARIABLES
 **********************/
static const ili9341_config_t *ili9341_config = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

esp_err_t ili9341_init(const ili9341_config_t *config) {
  if (config == NULL) {
      return ESP_ERR_INVALID_ARG;
  }
  ili9341_config = config;

	lcd_init_cmd_t ili_init_cmds[] = {
		{0xCF, {0x00, 0x83, 0X30}, 3},
		{0xED, {0x64, 0x03, 0X12, 0X81}, 4},
		{0xE8, {0x85, 0x01, 0x79}, 3},
		{0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
		{0xF7, {0x20}, 1},
		{0xEA, {0x00, 0x00}, 2},
		{0xC0, {0x26}, 1},          /*Power control*/
		{0xC1, {0x11}, 1},          /*Power control */
		{0xC5, {0x35, 0x3E}, 2},    /*VCOM control*/
		{0xC7, {0xBE}, 1},          /*VCOM control*/
		{0x36, {0x28}, 1},          /*Memory Access Control*/
		{0x3A, {0x55}, 1},			    /*Pixel Format Set*/
		{0xB1, {0x00, 0x1B}, 2},
		{0xF2, {0x08}, 1},
		{0x26, {0x01}, 1},
		{0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
		{0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
		{0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
		{0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
		{0x2C, {0}, 0},
		{0xB7, {0x07}, 1},
		{0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
		{0x11, {0}, 0x80},
		{0x29, {0}, 0x80},
		{0, {0}, 0xff},
	};

	// Initialize non-SPI GPIOs
  esp_rom_gpio_pad_select_gpio(config->dc_pin);
	gpio_set_direction(config->dc_pin, GPIO_MODE_OUTPUT);

	if (GPIO_IS_VALID_OUTPUT_GPIO(config->rst_pin)) {
    esp_rom_gpio_pad_select_gpio(config->rst_pin);
		gpio_set_direction(config->rst_pin, GPIO_MODE_OUTPUT);

		//  the display
		gpio_set_level(config->rst_pin, 0);
		vTaskDelay(100 / portTICK_PERIOD_MS);
		gpio_set_level(config->rst_pin, 1);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	esp3d_log("Initialization.");

	// Send all the commands
	uint16_t cmd = 0;
	while (ili_init_cmds[cmd].databytes!=0xff) {
		ili9341_send_cmd(ili_init_cmds[cmd].cmd);
		ili9341_send_data(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes&0x1F);
		if (ili_init_cmds[cmd].databytes & 0x80) {
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}
		cmd++;
	}

	return ESP_OK;
}

void ili9341_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data) {
	uint8_t data[4];

	/* Column addresses */
	ili9341_send_cmd(0x2A);
	data[0] = (x_start >> 8) & 0xFF;
	data[1] = x_start & 0xFF;
	data[2] = ((x_end-1) >> 8) & 0xFF;
	data[3] = (x_end-1) & 0xFF;
	ili9341_send_data(data, 4);

	/* Page addresses */
	ili9341_send_cmd(0x2B);
	data[0] = (y_start >> 8) & 0xFF;
	data[1] = y_start & 0xFF;
	data[2] = ((y_end-1) >> 8) & 0xFF;
	data[3] = (y_end-1) & 0xFF;
	ili9341_send_data(data, 4);

	/* Memory write */
	ili9341_send_cmd(0x2C);
	uint32_t size = (x_end - x_start) * (y_end - y_start) * 2;	
	ili9341_send_color((void*)color_data, size);	
}

void ili9341_sleep_in() {
	uint8_t data[] = {0x08};
	ili9341_send_cmd(0x10);
	ili9341_send_data(&data, 1);
}

void ili9341_sleep_out() {
	uint8_t data[] = {0x08};
	ili9341_send_cmd(0x11);
	ili9341_send_data(&data, 1);
}

void ili9341_set_invert_color(bool invert) {
	if (invert)
 		ili9341_send_cmd(0x21);
	else
	 	ili9341_send_cmd(0x20);
}

void ili9341_set_orientation(uint8_t orientation) {
    // ESP_ASSERT(orientation < 4);
#if ESP3D_TFT_LOG
    const char *orientation_str[] = {
        "PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"
    };
#endif //ESP3D_TFT_LOG
    esp3d_log("Display orientation: %s", orientation_str[orientation]);

    uint8_t data[] = {0x48, 0x88, 0x28, 0xE8};

    esp3d_log("0x36 command value: 0x%02X", data[orientation]);

    ili9341_send_cmd(0x36);
    ili9341_send_data((void *) &data[orientation], 1);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void ili9341_send_cmd(uint8_t cmd) {
    disp_wait_for_pending_transactions();
    gpio_set_level(ili9341_config->dc_pin, 0);	 /*Command mode*/
    disp_spi_send_data(&cmd, 1);
}

static void ili9341_send_data(void * data, uint16_t length) {
    disp_wait_for_pending_transactions();
    gpio_set_level(ili9341_config->dc_pin, 1);	 /*Data mode*/
    disp_spi_send_data(data, length);
}

static void ili9341_send_color(void * data, uint16_t length) {
    disp_wait_for_pending_transactions();
    gpio_set_level(ili9341_config->dc_pin, 1);   /*Data mode*/
    disp_spi_send_colors(data, length);
}
