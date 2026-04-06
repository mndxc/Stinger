#ifndef DISPLAY_H
#define DISPLAY_H

#include "esp_err.h"

/**
 * @brief Initialize the E-Paper display
 * @return esp_err_t ESP_OK on success
 */
esp_err_t display_init(void);

#endif // DISPLAY_H
