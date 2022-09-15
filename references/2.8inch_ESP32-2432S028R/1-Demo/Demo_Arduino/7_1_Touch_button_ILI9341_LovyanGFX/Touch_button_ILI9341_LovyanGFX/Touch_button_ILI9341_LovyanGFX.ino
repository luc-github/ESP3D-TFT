/////////////////////////////////////////////////////////////////
/*
  ESP32 | LVGL8 | Ep 0. GFX Setup (ft. LovyanGFX)
  Video Tutorial: https://youtu.be/IPCvQ4o_WP8
  Created by Eric N. (ThatProject) 
*/
/////////////////////////////////////////////////////////////////
//To download the LovyanGFX library before use//
#include <lvgl.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
lgfx::Panel_ILI9341     _panel_instance;
lgfx::Bus_SPI       _bus_instance;   // SPI bus instance // SPIバスのインスタンス
lgfx::Light_PWM     _light_instance;
lgfx::Touch_XPT2046     _touch_instance;

public:
  LGFX(void)
  {
    { // バス制御の設定を行います。
      auto cfg = _bus_instance.config();    // Gets the structure for bus configuration. // バス設定用の構造体を取得します。
      cfg.spi_host = HSPI_HOST;     // Select the SPI to use // 使用するSPIを選択  (VSPI_HOST or HSPI_HOST)
      cfg.spi_mode = 0;             // Set SPI communication mode (0 ~ 3) // SPI通信モードを設定 (0 ~ 3)
      cfg.freq_write = 55000000;    // SPI clock at the time of transmission (up to 80MHz, rounded to the value obtained by dividing 80MHz by an integer) // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  = 20000000;    // SPI clock when receiving // 受信時のSPIクロック
      cfg.spi_3wire  = false;       // Set true when receiving with MOSI pin // 受信をMOSIピンで行う場合はtrueを設定
      cfg.use_lock   = true;        // Set to true when using transaction lock // トランザクションロックを使用する場合はtrueを設定
      cfg.dma_channel = 1;          // Set the DMA channel (1 or 2. 0=disable) // 使用するDMAチャンネルを設定 (0=DMA不使用)
      cfg.pin_sclk = 14;            // Set SPI SCLK pin number // SPIのSCLKピン番号を設定
      cfg.pin_mosi = 13;            // Set SPI MOSI pin number // SPIのMOSIピン番号を設定
      cfg.pin_miso = 12;            // Set SPI MISO pin number (-1 = disable) // SPIのMISOピン番号を設定 (-1 = disable)
      cfg.pin_dc   = 2;             // Set SPI D / C pin number (-1 = disable) // SPIのD/Cピン番号を設定  (-1 = disable)

      _bus_instance.config(cfg);    // The set value is reflected on the bus. // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance);      // Set the bus on the panel. // バスをパネルにセットします。
    }

    { // Set the display panel control.//表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // Gets the structure for display panel settings.// 表示パネル設定用の構造体を取得します。
      cfg.pin_cs           =    15;  // Pin number to which CS is connected (-1 = disable) // CSが接続されているピン番号   (-1 = disable)
      cfg.pin_rst          =    -1;  // Pin number to which RST is connected (-1 = disable) // RSTが接続されているピン番号  (-1 = disable)
      cfg.pin_busy         =    -1;  // Pin number to which BUSY is connected (-1 = disable) // BUSYが接続されているピン番号 (-1 = disable)
      cfg.memory_width     =   240;  // Maximum width supported by driver IC // ドライバICがサポートしている最大の幅
      cfg.memory_height    =   320;  // Maximum height supported by driver IC // ドライバICがサポートしている最大の高さ
      cfg.panel_width      =   240;  // Actually displayable width // 実際に表示可能な幅
      cfg.panel_height     =   320;  // Actually displayable height // 実際に表示可能な高さ
      cfg.offset_x         =     0;  // Amount of X-direction offset of the panel // パネルのX方向オフセット量
      cfg.offset_y         =     0;  // Amount of Y-direction offset of the panel // パネルのY方向オフセット量
      cfg.offset_rotation  =     0;  // Offset of values in the direction of rotation 0 ~ 7 (4 ~ 7 are upside down) // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel =     8;  // Number of dummy read bits before pixel reading // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits  =     1;  // Number of bits of dummy read before reading data other than pixels // ピクセル以外のデータ読出し前のダミーリードのビット数
      cfg.readable         =  true;  // Set to true if data can be read // データ読出しが可能な場合 trueに設定
      cfg.invert           = false;  // Set to true if the light and darkness of the panel is reversed // パネルの明暗が反転してしまう場合 trueに設定
      cfg.rgb_order        = false;  // Set to true if the red and blue of the panel are swapped // パネルの赤と青が入れ替わってしまう場合 trueに設定
      cfg.dlen_16bit       = false;  // Set to true for panels that send data length in 16-bit units // データ長を16bit単位で送信するパネルの場合 trueに設定
      cfg.bus_shared       =  true;  // If the bus is shared with the SD card, set to true (bus control is performed with drawJpgFile etc.) // SDカードとバスを共有している場合 trueに設定(drawJpgFile等でバス制御を行います)

      _panel_instance.config(cfg);
    }
    
    { // Set the backlight control. (Delete if not needed // バックライト制御の設定を行います。（必要なければ削除）
      auto cfg = _light_instance.config();    // Gets the structure for the backlight setting. // バックライト設定用の構造体を取得します。

      cfg.pin_bl = 21;              // Pin number to which the backlight is connected // バックライトが接続されているピン番号
      cfg.invert = false;           // True if you want to invert the brightness of the backlight // バックライトの輝度を反転させる場合 true
      cfg.freq   = 44100;           // Backlight PWM frequency // バックライトのPWM周波数
      cfg.pwm_channel = 7;          // PWM channel number to use // 使用するPWMのチャンネル番号

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // Set the backlight on the panel. // バックライトをパネルにセットします。
    }

    { // Set the touch screen control. (Delete if not needed) // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();
      cfg.x_min      = 0;    // Minimum X value (raw value) obtained from touch screen // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max      = 239;  // Maximum X value (raw value) obtained from the touch screen // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min      = 0;    // Minimum Y value (raw value) obtained from touch screen // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max      = 319;  // Maximum Y value (raw value) obtained from the touch screen // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int    = 36;   // Pin number to which INT is connected // INTが接続されているピン番号
      cfg.bus_shared = true; // Set to true if you are using the same bus as the screen // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0;// Adjustment when the display and touch orientation do not match Set with a value from 0 to 7 // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
      cfg.spi_host = VSPI_HOST;// Select the SPI to use (HSPI_HOST or VSPI_HOST) // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
      cfg.freq = 2500000;     // Set SPI clock // SPIクロックを設定
      cfg.pin_sclk = 25;     // Pin number to which SCLK is connected // SCLKが接続されているピン番号
      cfg.pin_mosi = 32;     // Pin number to which MOSI is connected // MOSIが接続されているピン番号
      cfg.pin_miso = 39;     // Pin number to which MISO is connected // MISOが接続されているピン番号
      cfg.pin_cs   = 33;     // Pin number to which CS is connected // CSが接続されているピン番号
      
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // Set the touch screen on the panel. // タッチスクリーンをパネルにセットします。
    }
    setPanel(&_panel_instance); // Set the panel to be used. // 使用するパネルをセットします。
  }
};

LGFX tft;

/*Change to your screen resolution*/
static const uint32_t screenWidth  = 240;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
   uint32_t w = ( area->x2 - area->x1 + 1 );
   uint32_t h = ( area->y2 - area->y1 + 1 );

   tft.startWrite();
   tft.setAddrWindow( area->x1, area->y1, w, h );
   //tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
   tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
   tft.endWrite();

   lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
   uint16_t touchX, touchY;
   bool touched = tft.getTouch( &touchX, &touchY);
   if( !touched )
   {
      data->state = LV_INDEV_STATE_REL;
   }
   else
   {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
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
   Serial.begin(115200);

   tft.begin();        
   tft.setRotation(2);
   tft.setBrightness(255);
   uint16_t calData[] = { 239, 3926, 233, 265, 3856, 3896, 3714, 308};
   tft.setTouchCalibrate(calData);
   //touch_calibrate();//屏幕校准
   lv_init();
   lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

   /*Initialize the display*/
   static lv_disp_drv_t disp_drv;
   lv_disp_drv_init(&disp_drv);

   /*Change the following line to your display resolution*/
   disp_drv.hor_res = screenWidth;
   disp_drv.ver_res = screenHeight;
   disp_drv.flush_cb = my_disp_flush;
   disp_drv.draw_buf = &draw_buf;
   lv_disp_drv_register(&disp_drv);

   /*Initialize the (dummy) input device driver*/
   static lv_indev_drv_t indev_drv;
   lv_indev_drv_init(&indev_drv);
   indev_drv.type = LV_INDEV_TYPE_POINTER;
   indev_drv.read_cb = my_touchpad_read;
   lv_indev_drv_register(&indev_drv);

   lv_example_get_started_1();
}

void loop()
{
   lv_timer_handler(); /* let the GUI do its work */
   delay( 5 );
}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

/**
 * Create a button with a label and react on click event.
 */
void lv_example_get_started_1(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_align(btn, LV_ALIGN_CENTER, 0,0);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);
}
