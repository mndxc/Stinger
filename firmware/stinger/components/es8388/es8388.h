#ifndef ES8388_H
#define ES8388_H

#include "esp_err.h"

/**
 * @brief Initialize the ES8388 audio codec
 * @return esp_err_t ESP_OK on success
 */
esp_err_t es8388_init(void);

#endif // ES8388_H
