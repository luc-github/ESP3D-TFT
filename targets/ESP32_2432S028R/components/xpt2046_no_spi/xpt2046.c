/**
 * @file XPT2046.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "xpt2046.h"
#include "rom/ets_sys.h"
#include "esp_system.h"
#include "esp3d_log.h"
#include "driver/gpio.h"
#include "touch_def.h"
#include <stddef.h>

/*********************
 *      DEFINES
 *********************/

#define CMD_X_READ  0b10010000  // NOTE: XPT2046 data sheet says this is actually Y
#define CMD_Y_READ  0b11010000  // NOTE: XPT2046 data sheet says this is actually X

#define XPT2046_SMP_SIZE 4

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
static void xpt2046_corr(int16_t * x, int16_t * y);
static xpt2046_touch_detect_t xpt2048_is_touch_detected();

static uint16_t TP_Read_XOY(uint8_t xy);
static bool TP_Read_XY(uint16_t *x,uint16_t *y);
static void xpt2046_gpio_Write_Byte(uint8_t data);
static uint16_t xpt2046_gpio_spi_read_reg(uint8_t reg);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the XPT2046
 */
void xpt2046_init(void)
{

    esp3d_log( "XPT2046 Initialization");
    esp_err_t ret;
#if XPT2046_TOUCH_IRQ || XPT2046_TOUCH_IRQ_PRESS
    gpio_config_t irq_config = {
        .pin_bit_mask = BIT64(XPT2046_TOUCH_IRQ),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ret = gpio_config(&irq_config);
    assert(ret == ESP_OK);
#endif

    esp3d_log( "Pins configuration (Miso)");
    gpio_config_t miso_config = {
        .pin_bit_mask = BIT64(TOUCH_SPI_MISO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&miso_config);
    assert(ret == ESP_OK);

    esp3d_log( "Pins configuration (Mosi)");
    gpio_pad_select_gpio(TOUCH_SPI_MOSI);
    gpio_set_direction(TOUCH_SPI_MOSI, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(TOUCH_SPI_CLK);
    gpio_set_direction(TOUCH_SPI_CLK, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(TOUCH_SPI_CS);
    gpio_set_direction(TOUCH_SPI_CS, GPIO_MODE_OUTPUT);
    assert(ret == ESP_OK);
}

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 */
void xpt2046_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    static int16_t last_x = 0;
    static int16_t last_y = 0;
    bool valid = false;
    int16_t x = last_x;
    int16_t y = last_y;
    uint16_t ux = 0;
    uint16_t uy = 0;


    if (xpt2048_is_touch_detected() == TOUCH_DETECTED) {
        if(TP_Read_XY(&ux,&uy)) {
            esp3d_log("XPT2046 Read raw: x:%d   y:%d", ux, uy);
            if(ux > 350) {
                ux -= 350;
            } else {
                ux = 0;
            }
            if(uy > 190) {
                uy -= 190;
            } else {
                uy = 0;
            }
			//I do not get what is that calculation ....
			//come from 
            ux = 20+(uint32_t)((uint32_t)ux * LV_HOR_RES) / (3870 - 350);//320   3870-350
            uy = (uint32_t)((uint32_t)uy * LV_VER_RES) / (3870 - 190);// 240  3870-190
            x = ux;
            y = uy;
            xpt2046_corr(&x, &y);
            esp3d_log("x = %d, y = %d", x, y);
            last_x = x;
            last_y = x;
            valid = true;
        } 
    } 
    data->point.x = x;
    data->point.y = y;
    data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

uint16_t TP_Read_XOY(uint8_t xy)
{
    uint8_t READ_TIMES = 30;
    uint8_t LOST_VAL = 1;
    uint16_t i, j,temp,buf[READ_TIMES];
    uint32_t sum=0;
    for(i=0; i<READ_TIMES; i++) {
        buf[i]=xpt2046_gpio_spi_read_reg(xy);
    }
    for(i=0; i<READ_TIMES-1; i++) {
        for(j=i+1; j<READ_TIMES; j++) {
            if(buf[i]>buf[j]) {
                temp=buf[i];
                buf[i]=buf[j];
                buf[j]=temp;
            }
        }
    }
    sum=0;
    for(i=LOST_VAL; i<READ_TIMES-LOST_VAL; i++) {
        sum+=buf[i];
    }
    temp = sum/(READ_TIMES-2*LOST_VAL);
    return temp;
}


//Check if reading is acceptable
//check 2 time position, if in range then ok
#define ERR_RANGE 50
bool TP_Read_XY(uint16_t *x,uint16_t *y)
{
	for (int i = 0; i < XPT2046_SMP_SIZE; i++) {
    uint16_t x1 = TP_Read_XOY(CMD_X_READ);
	uint16_t y1 = TP_Read_XOY(CMD_Y_READ);
	uint16_t x2 = TP_Read_XOY(CMD_X_READ);
	uint16_t y2 = TP_Read_XOY(CMD_Y_READ);

    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))
            &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE))) {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return true;
    }
	//esp3d_log ("x1:%d y1:%d x2:%d y2:%d",x1,y1,x2,y2);
	}
    return false;
}

static void xpt2046_corr(int16_t * x, int16_t * y)
{
#if XPT2046_XY_SWAP != 0
    int16_t swap_tmp;
    swap_tmp = *x;
    *x = *y;
    *y = swap_tmp;
#endif

#if XPT2046_X_INV != 0
    (*x) =  LV_HOR_RES - (*x);
#endif

#if XPT2046_Y_INV != 0
    (*y) =  LV_VER_RES - (*y);
#endif
}

static xpt2046_touch_detect_t xpt2048_is_touch_detected()
{

#if XPT2046_TOUCH_IRQ || XPT2046_TOUCH_IRQ_PRESS
    uint8_t irq = gpio_get_level(XPT2046_TOUCH_IRQ);

    if (irq != 0) {
        return TOUCH_NOT_DETECTED;
    }
#endif

    return TOUCH_DETECTED;
}

//SPI Command to write byte
void xpt2046_gpio_Write_Byte(uint8_t data)
{
    uint8_t i=0;
    for(i=0; i<8; i++) {
        if(data&0x80) {
            gpio_set_level(TOUCH_SPI_MOSI, 1);
        } else {
            gpio_set_level(TOUCH_SPI_MOSI, 0);
        }
        data<<=1;
        gpio_set_level(TOUCH_SPI_CLK, 0);
        gpio_set_level(TOUCH_SPI_CLK, 1);
    }
}

//SPI Command to read register
uint16_t xpt2046_gpio_spi_read_reg(uint8_t reg)
{
    uint8_t count=0;
    uint16_t Data=0;
    gpio_set_level(TOUCH_SPI_CLK, 0);
    gpio_set_level(TOUCH_SPI_MOSI, 0);
    gpio_set_level(TOUCH_SPI_CS, 0);

    xpt2046_gpio_Write_Byte(reg);

    ets_delay_us(6);

    gpio_set_level(TOUCH_SPI_CLK, 0);
    ets_delay_us(1);
    gpio_set_level(TOUCH_SPI_CLK, 1);
    gpio_set_level(TOUCH_SPI_CLK, 0);
    for(count=0; count<16; count++) {
        Data<<=1;
        gpio_set_level(TOUCH_SPI_CLK, 0);
        gpio_set_level(TOUCH_SPI_CLK, 1);
        if(gpio_get_level(TOUCH_SPI_MISO)) {
            Data++;
        }
    }
    Data>>=4;
    gpio_set_level(TOUCH_SPI_CS, 1);
    return(Data);
}