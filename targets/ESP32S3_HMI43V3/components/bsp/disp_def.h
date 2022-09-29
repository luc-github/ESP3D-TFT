//Pins for ESP32S3 HMI43V3
//Display driver RM68120 parallele 8080
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#define TFT_DISPLAY_CONTROLLER "RM68120"

#define DISP_DIRECTION_LANDSCAPE   (1)  // 1:landscape mode   0:portrait mode

#if DISP_DIRECTION_LANDSCAPE == 1  // landscape mode
#define DISP_HOR_RES_MAX      (800)
#define DISP_VER_RES_MAX      (480)
#else   // portrait mode
#define DISP_HOR_RES_MAX      (480)
#define DISP_VER_RES_MAX      (800)
#endif

#define DISP_BUF_SIZE  (DISP_HOR_RES_MAX * 40)

#define DISP_BITS_WIDTH         (16)
#define DISP_CMD_BITS_WIDTH     (16)
#define DISP_PARAM_BITS_WIDTH   (16)
#define DISP_CLK_FREQ   (8000000)

#define DISP_BL_PIN     (-1)
#define DISP_CS_PIN     (-1)
#define DISP_RST_PIN    (21) //GPIO 21
#define DISP_RS_PIN     (38) //GPIO 38
#define DISP_WR_PIN     (17) //GPIO 17

#define DISP_D00_PIN    (1)  //GPIO 1
#define DISP_D01_PIN    (9)  //GPIO 9
#define DISP_D02_PIN    (2)  //GPIO 2
#define DISP_D03_PIN    (10) //GPIO 10
#define DISP_D04_PIN    (3)  //GPIO 3
#define DISP_D05_PIN    (11) //GPIO 11
#define DISP_D06_PIN    (4)  //GPIO 4
#define DISP_D07_PIN    (12) //GPIO 12
#define DISP_D08_PIN    (5)  //GPIO 5
#define DISP_D09_PIN    (13) //GPIO 13

#define DISP_D10_PIN    (6)  //GPIO 6
#define DISP_D11_PIN    (14) //GPIO 14
#define DISP_D12_PIN    (7)  //GPIO 7
#define DISP_D13_PIN    (15) //GPIO 15
#define DISP_D14_PIN    (8)  //GPIO 8
#define DISP_D15_PIN    (16) //GPIO 16

#ifdef __cplusplus
} /* extern "C" */
#endif
