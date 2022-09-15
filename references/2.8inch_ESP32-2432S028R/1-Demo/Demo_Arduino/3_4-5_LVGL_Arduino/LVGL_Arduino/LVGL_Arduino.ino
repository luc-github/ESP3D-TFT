#include <lvgl.h>
#include <TFT_eSPI.h>

#include <demos/lv_demos.h>

/*更改屏幕分辨率*/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

TFT_eSPI tft = TFT_eSPI(); /* TFT实例 */

#if LV_USE_LOG != 0
/* 串行调试 */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif
//_______________________
void lv_example_btn(void)
{
    /*要转换的属性*/
    static lv_style_prop_t props[] = {
            LV_STYLE_TRANSFORM_WIDTH, LV_STYLE_TRANSFORM_HEIGHT, LV_STYLE_TEXT_LETTER_SPACE};

    /*Transition descriptor when going back to the default state.
     *Add some delay to be sure the press transition is visible even if the press was very short*/
    static lv_style_transition_dsc_t transition_dsc_def;
    lv_style_transition_dsc_init(&transition_dsc_def, props, lv_anim_path_overshoot, 250, 100, NULL);

    /*Transition descriptor when going to pressed state.
     *No delay, go to presses state immediately*/
    static lv_style_transition_dsc_t transition_dsc_pr;
    lv_style_transition_dsc_init(&transition_dsc_pr, props, lv_anim_path_ease_in_out, 250, 0, NULL);

    /*Add only the new transition to he default state*/
    static lv_style_t style_def;
    lv_style_init(&style_def);
    lv_style_set_transition(&style_def, &transition_dsc_def);

    /*Add the transition and some transformation to the presses state.*/
    static lv_style_t style_pr;
    lv_style_init(&style_pr);
    lv_style_set_transform_width(&style_pr, 10);
    lv_style_set_transform_height(&style_pr, -10);
    lv_style_set_text_letter_space(&style_pr, 10);
    lv_style_set_transition(&style_pr, &transition_dsc_pr);

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -80);
    lv_obj_add_style(btn1, &style_pr, LV_STATE_PRESSED);
    lv_obj_add_style(btn1, &style_def, 0);

    lv_obj_t * label = lv_label_create(btn1);
    lv_label_set_text(label, "btn1");

        /*Init the style for the default state*/
    static lv_style_t style;
    lv_style_init(&style);

    lv_style_set_radius(&style, 3);

    lv_style_set_bg_opa(&style, LV_OPA_100);
    lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_color(&style, lv_palette_darken(LV_PALETTE_BLUE, 2));
    lv_style_set_bg_grad_dir(&style, LV_GRAD_DIR_VER);

    lv_style_set_border_opa(&style, LV_OPA_40);
    lv_style_set_border_width(&style, 2);
    lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_GREY));

    lv_style_set_shadow_width(&style, 8);
    lv_style_set_shadow_color(&style, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_shadow_ofs_y(&style, 8);

    lv_style_set_outline_opa(&style, LV_OPA_COVER);
    lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_BLUE));

    lv_style_set_text_color(&style, lv_color_white());
    lv_style_set_pad_all(&style, 10);

    /*Init the pressed style*/
    static lv_style_t style_pr_2;
    lv_style_init(&style_pr_2);

    /*Ad a large outline when pressed*/
    lv_style_set_outline_width(&style_pr_2, 30);
    lv_style_set_outline_opa(&style_pr_2, LV_OPA_TRANSP);

    lv_style_set_translate_y(&style_pr_2, 5);
    lv_style_set_shadow_ofs_y(&style_pr_2, 3);
    lv_style_set_bg_color(&style_pr_2, lv_palette_darken(LV_PALETTE_BLUE, 2));
    lv_style_set_bg_grad_color(&style_pr_2, lv_palette_darken(LV_PALETTE_BLUE, 4));

    /*Add a transition to the the outline*/
    static lv_style_transition_dsc_t trans;
    static lv_style_prop_t props2[] = {LV_STYLE_OUTLINE_WIDTH, LV_STYLE_OUTLINE_OPA};
    lv_style_transition_dsc_init(&trans, props2, lv_anim_path_linear, 300, 0, NULL);

    lv_style_set_transition(&style_pr_2, &trans);

    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
    lv_obj_remove_style_all(btn2);                          /*Remove the style coming from the theme*/
    lv_obj_add_style(btn2, &style, 0);
    lv_obj_add_style(btn2, &style_pr_2, LV_STATE_PRESSED);
    lv_obj_set_size(btn2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(btn2);

    lv_obj_t * label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Button");
    lv_obj_center(label2);
}
//_______________________
/* 显示器刷新 */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

/*读取触摸板*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*设置坐标*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}

void setup()
{
    Serial.begin( 115200 ); /*初始化串口*/

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* 用于调试的注册打印功能 */
#endif

    tft.begin();          /*初始化*/
    tft.setRotation(1); /* 旋转 */

    //校准模式。一是四角定位、二是直接输入模拟数值直接定位
    //touch_calibrate();//屏幕校准
    uint16_t calData[5] = { 145, 3788, 271, 3535, 1 };
    tft.setTouch( calData );

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

    /*初始化显示*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*将以下行更改为显示分辨率*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*初始化（虚拟）输入设备驱动程序*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

#if 0
    /* 创建简单标签 */
//    lv_obj_t *label = lv_label_create( lv_scr_act() );
//    lv_label_set_text( label, LVGL_Arduino.c_str() );
//    lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
     lv_example_btn();
#else
    /* 尝试lv_examples Arduino库中的一个示例
       请确保按照上面所写的内容将其包括在内。
    lv_example_btn_1();
   */

    // uncomment one of these demos
    lv_demo_widgets();            // OK
    // lv_demo_benchmark();          // OK
    // lv_demo_keypad_encoder();     // works, but I haven't an encoder
    // lv_demo_music();              // NOK
    // lv_demo_printer();
    // lv_demo_stress();             // seems to be OK
#endif
    Serial.println( "Setup done" );
}

void loop()
{
    lv_timer_handler(); /* 让GUI完成它的工作 */
    delay( 5 );
}

void touch_calibrate()//屏幕校准
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  //校准
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("按指示触摸角落");

  tft.setTextFont(1);
  tft.println();

  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  Serial.println(); Serial.println();
  Serial.println("//在setup()中使用此校准代码:");
  Serial.print("uint16_t calData[5] = ");
  Serial.print("{ ");

  for (uint8_t i = 0; i < 5; i++)
  {
    Serial.print(calData[i]);
    if (i < 4) Serial.print(", ");
  }

  Serial.println(" };");
  Serial.print("  tft.setTouch(calData);");
  Serial.println(); Serial.println();

  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("XZ OK!");
  tft.println("Calibration code sent to Serial port.");

}
