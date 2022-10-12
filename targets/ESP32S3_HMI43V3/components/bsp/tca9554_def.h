#ifndef _BSP_EXT_IO_H_
#define _BSP_EXT_IO_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TCA9554_CLK_SPEED               (400000)
#define TCA9554A_ADDR                    (0x24)
#define TCA9554_ADDR                     (0x3C)

#define TCA9554_INPUT_PORT_REG          (0x00)
#define TCA9554_OUTPUT_PORT_REG         (0x01)
#define TCA9554_POLARITY_INVERSION_REG  (0x02)
#define TCA9554_CONFIGURATION_REG       (0x03)

typedef struct {
    union {
        struct {
            uint8_t ext_io0:1;
            uint8_t ext_io1:1;
            uint8_t ext_io2:1;
            uint8_t ext_io3:1;
            uint8_t lcd_bl:1;
            uint8_t audio_pa:1;
            uint8_t boost_en:1;
            uint8_t tp_rst:1;
        };
        uint8_t val;
    };
} ext_io_t;

#define BSP_EXT_IO_DEFAULT_CONFIG() {   \
        .ext_io0 = 1,                   \
        .ext_io1 = 1,                   \
        .ext_io2 = 1,                   \
        .ext_io3 = 1,                   \
        .lcd_bl = 0,                    \
        .audio_pa = 0,                  \
        .boost_en = 0,                  \
        .tp_rst = 0,                    \
    }

#define BSP_EXT_IO_DEFAULT_LEVEL() {    \
        .ext_io0 = 0,                   \
        .ext_io1 = 0,                   \
        .ext_io2 = 0,                   \
        .ext_io3 = 0,                   \
        .lcd_bl = 1,                    \
        .audio_pa = 1,                  \
        .boost_en = 0,                  \
        .tp_rst = 1,                    \
    }

#define BSP_EXT_IO_SLEEP_LEVEL() {      \
        .ext_io0 = 0,                   \
        .ext_io1 = 0,                   \
        .ext_io2 = 0,                   \
        .ext_io3 = 0,                   \
        .lcd_bl = 0,                    \
        .audio_pa = 0,                  \
        .boost_en = 0,                  \
        .tp_rst = 0,                    \
    }

#define BSP_EXT_IO_OUTPUT_CONFIG() {   \
        .ext_io0 = 0,                   \
        .ext_io1 = 0,                   \
        .ext_io2 = 0,                   \
        .ext_io3 = 0,                   \
        .lcd_bl = 0,                    \
        .audio_pa = 0,                  \
        .boost_en = 0,                  \
        .tp_rst = 0,                    \
    }

#define BSP_EXT_IO_OUTPUT_DEFAULT_LEVEL() {    \
        .ext_io0 = 0,                   \
        .ext_io1 = 0,                   \
        .ext_io2 = 0,                   \
        .ext_io3 = 0,                   \
        .lcd_bl = 1,                    \
        .audio_pa = 1,                  \
        .boost_en = 0,                  \
        .tp_rst = 1,                    \
    }

#endif
