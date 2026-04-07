#ifndef ES8388_H
#define ES8388_H

#include "esp_err.h"

/* 
 * ES8388 Register Map 
 */
#define ES8388_CONTROL1         0x00
#define ES8388_CONTROL2         0x01
#define ES8388_CHIPPOWER        0x02
#define ES8388_ADC_CONTROL1     0x03
#define ES8388_ADC_CONTROL2     0x04
#define ES8388_ADC_CONTROL3     0x05
#define ES8388_ADC_CONTROL4     0x06
#define ES8388_ADC_CONTROL5     0x07
#define ES8388_DAC_CONTROL1     0x08
#define ES8388_DAC_CONTROL2     0x09
#define ES8388_DAC_CONTROL3     0x0a
#define ES8388_DAC_CONTROL4     0x0b
#define ES8388_DAC_CONTROL5     0x0c
#define ES8388_DAC_CONTROL6     0x0d
#define ES8388_DAC_CONTROL7     0x0e
#define ES8388_DAC_CONTROL8     0x0f
#define ES8388_DAC_CONTROL9     0x10
#define ES8388_DAC_CONTROL10    0x11
#define ES8388_DAC_CONTROL11    0x12
#define ES8388_DAC_CONTROL12    0x13
#define ES8388_DAC_CONTROL13    0x14
#define ES8388_DAC_CONTROL14    0x15
#define ES8388_DAC_CONTROL15    0x16
#define ES8388_DAC_CONTROL16    0x17
#define ES8388_DAC_CONTROL17    0x18
#define ES8388_DAC_CONTROL18    0x19
#define ES8388_DAC_CONTROL19    0x1a
#define ES8388_DAC_CONTROL20    0x1b
#define ES8388_DAC_CONTROL21    0x1c
#define ES8388_DAC_CONTROL22    0x1d
#define ES8388_DAC_CONTROL23    0x1e
#define ES8388_DAC_CONTROL24    0x1f
#define ES8388_DAC_CONTROL25    0x20
#define ES8388_DAC_CONTROL26    0x21
#define ES8388_DAC_CONTROL27    0x22
#define ES8388_DAC_CONTROL28    0x23
#define ES8388_DAC_CONTROL29    0x24
#define ES8388_DAC_CONTROL30    0x25
#define ES8388_MASTER_MODE      0x08 

/**
 * @brief Initialize the ES8388 audio codec via I2C control bus.
 *        This also initializes the I2C master driver.
 * @return esp_err_t ESP_OK on success
 */
esp_err_t es8388_init(void);

/**
 * @brief Set the output volume for the headphones
 * @param volume 0-100 percentage
 * @return esp_err_t 
 */
esp_err_t es8388_set_voice_volume(int volume);

#endif // ES8388_H
