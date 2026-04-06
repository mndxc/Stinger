#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include "esp_err.h"

/**
 * @brief Mount SD card and initialize file manager
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sd_manager_init(void);

#endif // SD_MANAGER_H
