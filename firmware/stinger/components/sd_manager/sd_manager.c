#include "sd_manager.h"
#include "esp_log.h"

static const char *TAG = "SD_MANAGER";

esp_err_t sd_manager_init(void) {
    ESP_LOGI(TAG, "Initializing SD Manager...");
    // TODO: Implement SPI SD mount and mutex logic
    return ESP_OK;
}
