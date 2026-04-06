#include "input_handler.h"
#include "../../main/pin_map.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "input_handler";

static QueueHandle_t s_app_queue = NULL;

// Matrix Configuration
static const gpio_num_t matrix_rows[] = {
    PIN_MATRIX_ROW_0, PIN_MATRIX_ROW_1, PIN_MATRIX_ROW_2, PIN_MATRIX_ROW_3
};
static const gpio_num_t matrix_cols[] = {
    PIN_MATRIX_COL_0, PIN_MATRIX_COL_1, PIN_MATRIX_COL_2, PIN_MATRIX_COL_3
};

#define ROW_COUNT (sizeof(matrix_rows) / sizeof(gpio_num_t))
#define COL_COUNT (sizeof(matrix_cols) / sizeof(gpio_num_t))

// Previous state for debouncing
static bool last_matrix_state[ROW_COUNT][COL_COUNT] = {false};

// Encoder pins
#define ENC1_A_PIN PIN_ENC1_A
#define ENC1_B_PIN PIN_ENC1_B
#define ENC2_A_PIN PIN_ENC2_A
#define ENC2_B_PIN PIN_ENC2_B

static void IRAM_ATTR encoder_isr_handler(void* arg)
{
    uint32_t encoder_id = (uint32_t)(uintptr_t)arg;
    int a_val, b_val;
    input_event_t evt;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (encoder_id == 1) {
        a_val = gpio_get_level(ENC1_A_PIN);
        b_val = gpio_get_level(ENC1_B_PIN);
    } else {
        a_val = gpio_get_level(ENC2_A_PIN);
        b_val = gpio_get_level(ENC2_B_PIN);
    }

    // Basic quadrature decoding on A edge
    if (a_val == 0) {
        evt.type = (b_val == 0) ? INPUT_EVENT_ENCODER_CCW : INPUT_EVENT_ENCODER_CW;
        evt.data.encoder.encoder_id = encoder_id;
        if (s_app_queue) {
            xQueueSendFromISR(s_app_queue, &evt, &xHigherPriorityTaskWoken);
        }
    }
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

static void input_scan_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Input scan task started.");
    
    while(1) {
        for (int r = 0; r < ROW_COUNT; r++) {
            // Set current row LOW
            gpio_set_level(matrix_rows[r], 0);

            // Small delay to let line settle (matrix capacitance)
            esp_rom_delay_us(10); 

            // Read columns
            for (int c = 0; c < COL_COUNT; c++) {
                // If column is LOW, button is pressed (due to pullup)
                bool pressed = (gpio_get_level(matrix_cols[c]) == 0);
                
                if (pressed != last_matrix_state[r][c]) {
                    last_matrix_state[r][c] = pressed;
                    
                    if (s_app_queue) {
                        input_event_t evt;
                        evt.type = pressed ? INPUT_EVENT_MATRIX_PRESS : INPUT_EVENT_MATRIX_RELEASE;
                        evt.data.matrix.row = r;
                        evt.data.matrix.col = c;
                        
                        xQueueSend(s_app_queue, &evt, 0);
                    }
                }
            }

            // Set current row back HIGH
            gpio_set_level(matrix_rows[r], 1);
        }
        
        // Spec: 1ms scan interval
        vTaskDelay(pdMS_TO_TICKS(1)); 
    }
}

void input_handler_init(QueueHandle_t app_queue)
{
    s_app_queue = app_queue;

    // --- Configure Matrix Rows (Outputs) ---
    for (int i = 0; i < ROW_COUNT; i++) {
        gpio_config_t r_cfg = {
            .pin_bit_mask = (1ULL << matrix_rows[i]),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&r_cfg);
        // Default rows to HIGH (inactive)
        gpio_set_level(matrix_rows[i], 1);
    }

    // --- Configure Matrix Cols (Inputs with Pull-up) ---
    for (int i = 0; i < COL_COUNT; i++) {
        gpio_config_t c_cfg = {
            .pin_bit_mask = (1ULL << matrix_cols[i]),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&c_cfg);
    }

    // --- Configure Encoders ---
    gpio_install_isr_service(0);

    const gpio_num_t enc_pins_a[] = {ENC1_A_PIN, ENC2_A_PIN};
    const gpio_num_t enc_pins_b[] = {ENC1_B_PIN, ENC2_B_PIN};
    
    for (int i = 0; i < 2; i++) {
        // A pin (Interrupt on falling edge)
        gpio_config_t enc_a_cfg = {
            .pin_bit_mask = (1ULL << enc_pins_a[i]),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_NEGEDGE
        };
        gpio_config(&enc_a_cfg);
        
        // B pin (Read only)
        gpio_config_t enc_b_cfg = {
            .pin_bit_mask = (1ULL << enc_pins_b[i]),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&enc_b_cfg);

        gpio_isr_handler_add(enc_pins_a[i], encoder_isr_handler, (void*)(uintptr_t)(i + 1));
    }

    // --- Start Matrix Task ---
    // Core 0, Priority 20
    xTaskCreatePinnedToCore(input_scan_task, "InputScanTask", 4096, NULL, 20, NULL, 0);

    ESP_LOGI(TAG, "Input handler initialized.");
}
