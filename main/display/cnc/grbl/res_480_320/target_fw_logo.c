#include "lvgl.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_TARGET_FW_LOGO
#define LV_ATTRIBUTE_IMG_TARGET_FW_LOGO
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST
    LV_ATTRIBUTE_IMG_TARGET_FW_LOGO uint8_t target_fw_logo_map[] = {
        0xfb, 0xfb, 0xfb, 0xff, /*Color of index 0*/
        0x9d, 0x9d, 0x9d, 0xff, /*Color of index 1*/
        0x5b, 0x5b, 0x5b, 0xff, /*Color of index 2*/
        0x02, 0x02, 0x02, 0xff, /*Color of index 3*/

        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff,
        0xfd, 0x1b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf8, 0x2f, 0xff, 0xff, 0xff, 0xff, 0xf4, 0x03, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x07,
        0xff, 0xff, 0xff, 0xff, 0xf0, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xff,
        0xf0, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x02, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x06,
        0xff, 0xff, 0xff, 0xff, 0xf0, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff,
        0xf0, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xe0, 0x06, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x02, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07,
        0xff, 0xff, 0xff, 0xff, 0xf0, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x06, 0xff, 0xff, 0xff, 0xff,
        0xf0, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x02, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x06,
        0xff, 0xff, 0xff, 0xff, 0xf0, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff,
        0xf0, 0x02, 0xff, 0xff, 0xff, 0xff, 0xea, 0xab, 0xff, 0xff, 0xff, 0xff,
        0xea, 0x66, 0xe0, 0x06, 0xa6, 0x6b, 0xff, 0xff, 0xf0, 0x02, 0xff, 0xff,
        0xff, 0xf9, 0x00, 0x01, 0xbf, 0xff, 0xff, 0xfe, 0x00, 0x00, 0xe0, 0x07,
        0x40, 0x00, 0x6f, 0xff, 0xf0, 0x02, 0xff, 0xff, 0xff, 0xd0, 0x00, 0x00,
        0x1b, 0xff, 0xff, 0xe0, 0x00, 0x00, 0xa0, 0x07, 0x40, 0x00, 0x07, 0xff,
        0xf0, 0x02, 0xff, 0xff, 0xff, 0x40, 0x00, 0x00, 0x02, 0xff, 0xff, 0x80,
        0x00, 0x00, 0xe0, 0x03, 0x80, 0x00, 0x00, 0xff, 0xf0, 0x02, 0xff, 0xff,
        0xfd, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00, 0xe0, 0x07,
        0x40, 0x00, 0x00, 0x7f, 0xf0, 0x02, 0xff, 0xff, 0xf8, 0x00, 0x05, 0x40,
        0x00, 0x7f, 0xfc, 0x00, 0x01, 0x95, 0xe0, 0x07, 0x99, 0x50, 0x00, 0x1f,
        0xf0, 0x02, 0xff, 0xff, 0xf0, 0x00, 0x7f, 0xf8, 0x00, 0x2f, 0xf4, 0x00,
        0x2f, 0xff, 0xe0, 0x06, 0xff, 0xfd, 0x00, 0x0b, 0xf0, 0x02, 0xff, 0xff,
        0xe0, 0x01, 0xff, 0xfe, 0x00, 0x1f, 0xf0, 0x01, 0xff, 0xff, 0xe0, 0x07,
        0xff, 0xff, 0x80, 0x0b, 0xf0, 0x02, 0xff, 0xff, 0xd0, 0x07, 0xff, 0xff,
        0x80, 0x0b, 0xe0, 0x02, 0xff, 0xff, 0xe0, 0x06, 0xff, 0xff, 0xd0, 0x07,
        0xf0, 0x02, 0xff, 0xff, 0xc0, 0x0b, 0xff, 0xff, 0xc0, 0x0b, 0xe0, 0x07,
        0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xe0, 0x02, 0xf0, 0x02, 0xff, 0xff,
        0xc0, 0x0b, 0xff, 0xff, 0xc0, 0x0b, 0xd0, 0x07, 0xff, 0xff, 0xe0, 0x06,
        0xff, 0xff, 0xf0, 0x02, 0xf0, 0x02, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xff,
        0xd0, 0x0b, 0xd0, 0x0b, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xf0, 0x02,
        0xf4, 0x02, 0xff, 0xff, 0xc0, 0x0b, 0xff, 0xff, 0xd0, 0x0b, 0xd0, 0x0b,
        0xff, 0xff, 0xe0, 0x02, 0xff, 0xff, 0xe0, 0x03, 0xf4, 0x02, 0xff, 0xff,
        0xd0, 0x07, 0xff, 0xff, 0xd0, 0x0b, 0xd0, 0x07, 0xff, 0xff, 0xf0, 0x02,
        0xff, 0xff, 0xe0, 0x03, 0xf4, 0x01, 0xff, 0xff, 0xd0, 0x02, 0xff, 0xff,
        0xd0, 0x0b, 0xd0, 0x0b, 0xff, 0xff, 0xf4, 0x00, 0xff, 0xff, 0x80, 0x07,
        0xf8, 0x00, 0xbf, 0xff, 0xe0, 0x00, 0xbf, 0xff, 0xd0, 0x0b, 0xd0, 0x0b,
        0xff, 0xff, 0xf8, 0x00, 0x6f, 0xfe, 0x00, 0x0b, 0xfc, 0x00, 0x2f, 0xff,
        0xf4, 0x00, 0x1a, 0xaf, 0xd0, 0x0b, 0xd0, 0x0b, 0xff, 0xff, 0xfc, 0x00,
        0x06, 0xa4, 0x00, 0x1f, 0xfe, 0x00, 0x02, 0xbf, 0xfc, 0x00, 0x00, 0x02,
        0xd0, 0x0b, 0xd0, 0x0b, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x2f,
        0xff, 0x40, 0x00, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x90, 0x0b, 0xd0, 0x0b,
        0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0xbf, 0xff, 0x90, 0x00, 0x07,
        0xff, 0xc0, 0x00, 0x00, 0x50, 0x0b, 0xd0, 0x0b, 0xff, 0xff, 0xff, 0xe0,
        0x00, 0x00, 0x02, 0xff, 0xff, 0xf4, 0x00, 0x02, 0xff, 0xf4, 0x00, 0x01,
        0xd0, 0x0b, 0xe0, 0x0b, 0xff, 0xff, 0xff, 0xfd, 0x00, 0x00, 0x1f, 0xff,
        0xff, 0xfd, 0x00, 0x0b, 0xff, 0xff, 0x95, 0x17, 0xd0, 0x0b, 0xf9, 0x6f,
        0xff, 0xff, 0xff, 0xff, 0xe4, 0x56, 0xff, 0xff, 0xff, 0xff, 0xe5, 0x6f,
        0xff, 0xff, 0xff, 0xff, 0xd0, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xd0, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x0b, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xd0, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x80, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x0f, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xfe, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe4,
        0x00, 0x2f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf4, 0x00, 0x00, 0x7f, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xd0, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
        0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x00, 0x1f, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xe0, 0x05, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xef,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
};

const lv_img_dsc_t target_fw_logo = {
    .header.cf = LV_IMG_CF_INDEXED_2BIT,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 80,
    .header.h = 53,
    .data_size = 1076,
    .data = target_fw_logo_map,
};
