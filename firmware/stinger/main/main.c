#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_map.h"
#include "board_config.h"
#include "stinger_app.h"

void app_main(void)
{
    printf("Stinger USB Audio Soundboard Initializing...\n");
    
    // Initialize Layer 3 application which coordinates all subsystems
    stinger_app_init();
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
