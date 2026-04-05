#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_map.h"
#include "board_config.h"

void app_main(void)
{
    printf("Stinger USB Audio Soundboard Initializing...\n");
    
    // TODO: Initialize components (SD, I2S, TinyUSB, E-Paper)
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
