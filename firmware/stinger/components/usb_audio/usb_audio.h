#ifndef USB_AUDIO_H
#define USB_AUDIO_H

#include "esp_err.h"

/**
 * @brief Initialize the TinyUSB stack (UAC1/MSC/HID)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t usb_audio_init(void);

#endif // USB_AUDIO_H
