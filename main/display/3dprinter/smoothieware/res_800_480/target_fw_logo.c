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
        0xa1, 0xa1, 0xa1, 0xff, /*Color of index 1*/
        0x62, 0x62, 0x62, 0xff, /*Color of index 2*/
        0x01, 0x01, 0x01, 0xff, /*Color of index 3*/

        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x1f,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xd0, 0x00, 0x00, 0x0b, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xd0, 0x00, 0x00, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x80, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x00, 0x00, 0x03,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
        0xbf, 0xff, 0xff, 0xff, 0x40, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 0xe0, 0x3f, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xe0, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
        0xff, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
        0x00, 0xbf, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xf9, 0x00,
        0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x1b, 0xff, 0xe4,
        0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xd0, 0x00, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf8, 0x00, 0x00, 0x02, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0xbf, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00,
        0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff,
        0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x55, 0x55, 0xa9, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x55, 0x00, 0x00, 0x06, 0xe4, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff,
        0xff, 0xff, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00,
        0x1f, 0x40, 0x00, 0x00, 0x00, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00,
        0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x02, 0xc0, 0x00, 0x00,
        0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf4, 0x00, 0x00, 0x00, 0x00, 0x20,
        0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff,
        0xff, 0xff, 0xfd, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xd0, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
        0x00, 0x00, 0x07, 0x40, 0x00, 0x1b, 0xfe, 0x50, 0x00, 0xe0, 0x00, 0x00,
        0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x1e, 0x00,
        0x01, 0xff, 0xff, 0xf9, 0x00, 0xf8, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xd0, 0x00, 0x00, 0xbd, 0x00, 0x07, 0xff, 0xff, 0xff,
        0x81, 0xfe, 0x00, 0x00, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0,
        0x00, 0x01, 0xfc, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xfb, 0xff, 0x80, 0x00,
        0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x07, 0xf8, 0x00,
        0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x00, 0x07, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x40, 0x00, 0x0f, 0xf8, 0x00, 0x2f, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf4, 0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00,
        0x00, 0x2f, 0xf4, 0x00, 0x2f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00,
        0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x00, 0x00, 0x7f, 0xf4, 0x00,
        0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x00, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xfc, 0x00, 0x00, 0xbf, 0xf8, 0x00, 0x1f, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x00, 0x00, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00,
        0x00, 0xff, 0xf8, 0x00, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40,
        0x00, 0x3f, 0xff, 0xff, 0xff, 0xe5, 0x40, 0x00, 0x01, 0xff, 0xfc, 0x00,
        0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x04, 0x6b, 0xff,
        0x94, 0x00, 0x00, 0x00, 0x02, 0xff, 0xfd, 0x00, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x00,
        0x03, 0xff, 0xfe, 0x00, 0x00, 0x2f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0x40,
        0x00, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x6f, 0xff,
        0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x07, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xe0,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0b, 0xff, 0xff, 0xfc,
        0x00, 0x00, 0x00, 0x5f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x0b, 0xff, 0xff, 0xff, 0x40, 0x00, 0x00, 0x06,
        0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x0b, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xe0,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff,
        0xfe, 0x00, 0x00, 0x00, 0x0b, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00,
        0x03, 0xff, 0xff, 0xd0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x07, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x40, 0x00, 0x00, 0xff, 0xff, 0xd0,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xe0, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x01,
        0x40, 0x00, 0x00, 0x00, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00,
        0x00, 0x2f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x02, 0xe9, 0x00, 0x00, 0x00,
        0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0x40,
        0x00, 0x00, 0x01, 0x6f, 0xff, 0xfa, 0x50, 0x00, 0x00, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xf0, 0x00, 0x0b, 0xff, 0x00, 0x00, 0x06, 0xbf, 0xff,
        0xff, 0xff, 0xf4, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8,
        0x00, 0x0b, 0xfe, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00,
        0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x00, 0x07, 0xfd, 0x00,
        0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x00, 0x00, 0x1f, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xfd, 0x00, 0x07, 0xf4, 0x00, 0x00, 0xbf, 0xff, 0xff,
        0xff, 0xff, 0xfe, 0x00, 0x00, 0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
        0x00, 0x07, 0xf0, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40,
        0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x00, 0x07, 0xc0, 0x00,
        0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x00, 0x00, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xfd, 0x00, 0x0b, 0x40, 0x00, 0x03, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfc,
        0x00, 0x0a, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
        0x00, 0x00, 0x35, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x0d, 0x00, 0x00,
        0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x00, 0x00, 0x60, 0x6f,
        0xff, 0xff, 0xff, 0xf0, 0x00, 0x1d, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x10, 0x06, 0xff, 0xff, 0xff, 0x90,
        0x00, 0x38, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00,
        0x00, 0x00, 0x50, 0x00, 0x2b, 0xff, 0xf9, 0x00, 0x00, 0x70, 0x00, 0x00,
        0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf4, 0x00, 0x00, 0x00, 0x50, 0x00,
        0x00, 0x15, 0x40, 0x00, 0x01, 0xd0, 0x00, 0x00, 0x00, 0x2f, 0xff, 0xff,
        0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x80, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
        0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
        0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0x40, 0x00, 0x00, 0x00, 0x1e, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xff,
        0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x06, 0xe4, 0x00, 0x00, 0x00, 0x07,
        0xd0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x5f, 0x54, 0x00, 0x05, 0xbf, 0x80, 0x00, 0x00, 0x00,
        0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x0b,
        0xff, 0xef, 0xff, 0xff, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff,
        0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x40, 0x1f, 0xff, 0xff, 0xff, 0xff,
        0xe0, 0x01, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00,
        0x00, 0x06, 0xe0, 0x2f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0x40, 0x00,
        0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x1f, 0xfe, 0x7f,
        0xff, 0xff, 0xff, 0xff, 0xfa, 0xbf, 0xe4, 0x00, 0x01, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xc0, 0x00, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xfd, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0,
        0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
        0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x2f, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf4, 0x7f, 0xff, 0xff, 0xff,
};

const lv_img_dsc_t target_fw_logo = {
    .header.cf = LV_IMG_CF_INDEXED_2BIT,
    .header.always_zero = 0,
    .header.reserved = 0,
    .header.w = 80,
    .header.h = 72,
    .data_size = 1456,
    .data = target_fw_logo_map,
};
