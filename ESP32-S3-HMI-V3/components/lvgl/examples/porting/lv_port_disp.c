/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "../../lvgl.h"

/*********************
 *      DEFINES
 *********************/
static const char *TAG = "lv_port_disp";
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    static lv_disp_drv_t disp_drv;      // contains callback functions

    ESP_LOGI(TAG, "init lvgl lcd display port");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = GPIO_LCD_RS,
        .wr_gpio_num = GPIO_LCD_WR,
        .data_gpio_nums = {
            GPIO_LCD_D00, GPIO_LCD_D01, GPIO_LCD_D02, GPIO_LCD_D03, 
            GPIO_LCD_D04, GPIO_LCD_D05, GPIO_LCD_D06, GPIO_LCD_D07, 
            GPIO_LCD_D08, GPIO_LCD_D09, GPIO_LCD_D10, GPIO_LCD_D11,
            GPIO_LCD_D12, GPIO_LCD_D13, GPIO_LCD_D14, GPIO_LCD_D15,
        },
        .bus_width = LCD_BIT_WIDTH,
        .max_transfer_bytes = LCD_WIDTH * 40 * sizeof(uint16_t)    //最大32640 *2, 受DMA限制 
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = GPIO_LCD_CS,
        .pclk_hz = 8000000, // 测试14MHz以上花屏
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .on_color_trans_done = notify_lvgl_flush_ready,        //Callback invoked when color data transfer has finished
        .user_ctx = &disp_drv,                    //User private data, passed directly to on_color_trans_done’s user_ctx
        .lcd_cmd_bits = 16,
        .lcd_param_bits = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = GPIO_LCD_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_rm68120(io_handle, &panel_config, &panel_handle));

    esp_lcd_panel_reset(panel_handle);   // LCD Reset
    esp_lcd_panel_init(panel_handle);    // LCD init
    // esp_lcd_panel_swap_xy(panel_handle, true);
    // esp_lcd_panel_mirror(panel_handle, true, false);

    /* initialize LVGL draw buffers */
    static lv_disp_draw_buf_t draw_buf; 
    lv_color_t *buf1 = heap_caps_malloc(LCD_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA);   /* MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT */
    assert(buf1);
#if 1
    lv_color_t *buf2 = heap_caps_malloc(LCD_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LCD_WIDTH * 40);
#else                       
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, LCD_WIDTH * 40);   /*Initialize the display buffer*/
#endif

    /* Register the display in LVGL */
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;
    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf;

    disp_drv.user_data = panel_handle;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) disp_drv->user_data;

    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}

/**
 * @brief Task to generate ticks for LVGL.
 * 
 * @param pvParam Not used. 
 */
static void lv_tick_inc_cb(void *data)
{
    uint32_t tick_inc_period_ms = *((uint32_t *) data);

    lv_tick_inc(tick_inc_period_ms);
}

/**
 * @brief Create tick task for LVGL.
 * 
 * @return esp_err_t 
 */
esp_err_t lv_port_tick_init(void)
{
    static const uint32_t tick_inc_period_ms = 5;
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = lv_tick_inc_cb,
            .arg = &tick_inc_period_ms,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "",     /* name is optional, but may help identify the timer when debugging */
            .skip_unhandled_events = true,
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    /* The timer has been created but is not running yet. Start the timer now */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, tick_inc_period_ms * 1000));

    return ESP_OK;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
