#include "stinger_app.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "input_handler.h"
#include "es8388.h"

static QueueHandle_t s_app_queue = NULL;

static void stinger_app_task(void *pvParameters) {
    input_event_t evt;
    while(1) {
        if (xQueueReceive(s_app_queue, &evt, portMAX_DELAY)) {
            switch(evt.type) {
                case INPUT_EVENT_MATRIX_PRESS:
                    printf("Key Pressed: Row %d, Col %d\n", evt.data.matrix.row, evt.data.matrix.col);
                    break;
                case INPUT_EVENT_MATRIX_RELEASE:
                    printf("Key Released: Row %d, Col %d\n", evt.data.matrix.row, evt.data.matrix.col);
                    break;
                case INPUT_EVENT_ENCODER_CW:
                    printf("Encoder %d CW\n", evt.data.encoder.encoder_id);
                    break;
                case INPUT_EVENT_ENCODER_CCW:
                    printf("Encoder %d CCW\n", evt.data.encoder.encoder_id);
                    break;
            }
        }
    }
}

void stinger_app_init(void) {
    printf("Initializing Stinger App Layer...\n");
    
    // Core Layer 1 Subsystems
    es8388_init();

    s_app_queue = xQueueCreate(20, sizeof(input_event_t));
    if (s_app_queue) {
        input_handler_init(s_app_queue);
        xTaskCreate(stinger_app_task, "StingerAppTask", 4096, NULL, 5, NULL);
    }
}
