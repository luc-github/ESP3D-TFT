#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_TARGET_FW_LOGO
#define LV_ATTRIBUTE_IMG_TARGET_FW_LOGO
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_TARGET_FW_LOGO uint8_t target_fw_logo_map[] = {
  0xf6, 0xf6, 0xf6, 0xff, 	/*Color of index 0*/
  0xa5, 0xa5, 0xa5, 0xff, 	/*Color of index 1*/
  0x63, 0x63, 0x63, 0xff, 	/*Color of index 2*/
  0x03, 0x03, 0x03, 0xff, 	/*Color of index 3*/

  0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf0, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf0, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x00, 0x2f, 0xff, 0xff, 0xff, 0xff, 0xf0, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf0, 
  0xff, 0xff, 0xeb, 0xff, 0xff, 0x40, 0x00, 0x1f, 0xff, 0xfe, 0xff, 0xff, 0xf0, 
  0xff, 0xff, 0xc1, 0xff, 0xfe, 0x00, 0x00, 0x1f, 0xff, 0xf4, 0x2f, 0xff, 0xf0, 
  0xff, 0xfe, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x0b, 0xff, 0x90, 0x1f, 0xff, 0xf0, 
  0xff, 0xf8, 0x00, 0x0b, 0xd4, 0x00, 0x00, 0x01, 0xbd, 0x00, 0x06, 0xff, 0xf0, 
  0xff, 0xe0, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf0, 
  0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 
  0xff, 0x80, 0x00, 0x00, 0x00, 0x04, 0x55, 0x54, 0x00, 0x00, 0x00, 0x7f, 0xf0, 
  0xff, 0xe0, 0x00, 0x00, 0x01, 0x40, 0x00, 0x02, 0x40, 0x00, 0x00, 0xbf, 0xf0, 
  0xff, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x01, 0xff, 0xf0, 
  0xff, 0xfc, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x50, 0x00, 0x07, 0xff, 0xf0, 
  0xff, 0xfd, 0x00, 0x00, 0x90, 0x06, 0xff, 0x80, 0x60, 0x00, 0x0b, 0xff, 0xf0, 
  0xff, 0xff, 0x40, 0x02, 0x80, 0x1f, 0xff, 0xf5, 0xb8, 0x00, 0x1f, 0xff, 0xf0, 
  0xff, 0xff, 0x00, 0x0b, 0x80, 0x2f, 0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xf0, 
  0xff, 0xfd, 0x00, 0x1f, 0x40, 0x7f, 0xff, 0xff, 0xff, 0x40, 0x0b, 0xff, 0xf0, 
  0xff, 0xfc, 0x00, 0x3f, 0x40, 0x2f, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xf0, 
  0xff, 0xf4, 0x00, 0x7f, 0x80, 0x2f, 0xff, 0xff, 0xff, 0xd0, 0x02, 0xff, 0xf0, 
  0xa5, 0x40, 0x00, 0xbf, 0x80, 0x0b, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x15, 0xb0, 
  0x00, 0x00, 0x00, 0xff, 0xd0, 0x02, 0xff, 0xff, 0xff, 0xf4, 0x00, 0x00, 0x10, 
  0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x2f, 0xff, 0xff, 0xf4, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0xff, 0xf4, 0x00, 0x01, 0xff, 0xff, 0xf4, 0x00, 0x00, 0x10, 
  0x00, 0x00, 0x01, 0xff, 0xfe, 0x00, 0x00, 0x6f, 0xff, 0xf4, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0xff, 0xff, 0xd0, 0x00, 0x06, 0xff, 0xf4, 0x00, 0x00, 0x10, 
  0x00, 0x00, 0x01, 0xff, 0xff, 0xf9, 0x00, 0x00, 0xff, 0xf4, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xd0, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x10, 
  0x40, 0x00, 0x00, 0xbf, 0xff, 0xff, 0xf9, 0x00, 0x2f, 0xf0, 0x00, 0x00, 0x10, 
  0xf5, 0x50, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x40, 0x0f, 0xd0, 0x00, 0x56, 0xf0, 
  0xff, 0xf4, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xd0, 0x0f, 0xc0, 0x02, 0xff, 0xf0, 
  0xff, 0xfc, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xd0, 0x0b, 0x40, 0x07, 0xff, 0xf0, 
  0xff, 0xfd, 0x00, 0x0b, 0xff, 0xff, 0xff, 0xe0, 0x0a, 0x00, 0x0b, 0xff, 0xf0, 
  0xff, 0xfe, 0x00, 0x02, 0xff, 0xff, 0xff, 0xd0, 0x0c, 0x00, 0x0f, 0xff, 0xf0, 
  0xff, 0xff, 0x00, 0x01, 0xaf, 0xff, 0xff, 0xc0, 0x08, 0x00, 0x1f, 0xff, 0xf0, 
  0xff, 0xfd, 0x00, 0x01, 0x07, 0xff, 0xff, 0x40, 0x14, 0x00, 0x0b, 0xff, 0xf0, 
  0xff, 0xf8, 0x00, 0x01, 0x00, 0x5b, 0xb5, 0x00, 0x20, 0x00, 0x03, 0xff, 0xf0, 
  0xff, 0xf4, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x01, 0xff, 0xf0, 
  0xff, 0xd0, 0x00, 0x00, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0xbf, 0xf0, 
  0xff, 0x80, 0x00, 0x00, 0x54, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x7f, 0xf0, 
  0xff, 0xc0, 0x00, 0x00, 0x16, 0x91, 0x16, 0xb4, 0x00, 0x00, 0x00, 0x7f, 0xf0, 
  0xff, 0xe0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x01, 0xff, 0xf0, 
  0xff, 0xfc, 0x00, 0x0b, 0x5b, 0xff, 0xff, 0xfd, 0x5e, 0x00, 0x07, 0xff, 0xf0, 
  0xff, 0xfe, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x90, 0x1f, 0xff, 0xf0, 
  0xff, 0xff, 0x92, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf4, 0x7f, 0xff, 0xf0, 
};

const lv_img_dsc_t target_fw_logo = {
  .header.cf = LV_IMG_CF_INDEXED_2BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 50,
  .header.h = 45,
  .data_size = 601,
  .data = target_fw_logo_map,
};