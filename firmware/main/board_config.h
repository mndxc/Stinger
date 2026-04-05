#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/**
 * @file board_config.h
 * @brief Master constants for the Stinger USB Audio Soundboard
 */

// Audio System
#define SAMPLE_RATE         48000
#define BIT_DEPTH           16
#define MCLK_FREQ           12288000 // 256 * 48kHz

// Codec Configuration
#define ES8388_I2C_ADDR     0x10     // CE pin LOW (Rev V37)

// SD Card
#define MAX_BANKS           99
#define MAX_SAMPLE_RATE     48000
#define MAX_SAMPLE_LEN_SEC  30

// USB Strings (Rev 1.7)
#define USB_MANUFACTURER    "JP+"
#define USB_PRODUCT         "Stinger"
#define USB_UAC1_NAME       "Stinger Audio"
#define USB_MSC_LABEL       "STINGER"

// Display
#define SPLASH_DURATION_MS  2000

#endif // BOARD_CONFIG_H
