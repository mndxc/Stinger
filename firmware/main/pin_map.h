#ifndef PIN_MAP_H
#define PIN_MAP_H

/**
 * @file pin_map.h
 * @brief Single source of truth for all GPIO assignments (Rev V37)
 */

// USB
#define GPIO_USB_DM         19
#define GPIO_USB_DP         20

// I2S (Audio Data)
#define GPIO_I2S_BCLK       38
#define GPIO_I2S_LRCK       39
#define GPIO_I2S_DOUT       40
#define GPIO_I2S_DIN        41
#define GPIO_I2S_MCLK       0   // CLK_OUT1 (256xFs)

// I2C (Control)
#define GPIO_I2C_SDA        4
#define GPIO_I2C_SCL        5

// SPI2 (MicroSD)
#define GPIO_SD_CS          10
#define GPIO_SD_MOSI        11
#define GPIO_SD_SCK         12
#define GPIO_SD_MISO        13
#define GPIO_SD_CD          26  // Card-detect switch

// SPI3 (E-Paper Display)
#define GPIO_EPD_CS         21
#define GPIO_EPD_DIN        16
#define GPIO_EPD_CLK        17
#define GPIO_EPD_DC         18
#define GPIO_EPD_RST        7
#define GPIO_EPD_BUSY       6

// 4x4 Matrix Rows
#define GPIO_MATRIX_R0      1
#define GPIO_MATRIX_R1      2
#define GPIO_MATRIX_R2      42
#define GPIO_MATRIX_R3      43

// 4x4 Matrix Columns
#define GPIO_MATRIX_C0      8
#define GPIO_MATRIX_C1      3
#define GPIO_MATRIX_C2      44
#define GPIO_MATRIX_C3      9

// Rotary Encoders
#define GPIO_ENC1_A         47
#define GPIO_ENC1_B         14
#define GPIO_ENC2_A         22
#define GPIO_ENC2_B         23

// Status RGB LED (WS2812)
#define GPIO_RGB_LED        48

#endif // PIN_MAP_H
