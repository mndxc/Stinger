#include "input_handler.h"
#include "esp_log.h"

static const char *TAG = "INPUT";

esp_err_t input_handler_init(void) {
    ESP_LOGI(TAG, "Initializing Input Handler...");
    // TODO: Implement GPIO matrix and encoder ISRs
    return ESP_OK;
}
