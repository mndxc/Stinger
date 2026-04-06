#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "esp_err.h"

/**
 * @brief Initialize input scanning (Matrix/Encoders)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t input_handler_init(void);

#endif // INPUT_HANDLER_H
