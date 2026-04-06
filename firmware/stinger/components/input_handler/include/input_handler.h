#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    INPUT_EVENT_MATRIX_PRESS,
    INPUT_EVENT_MATRIX_RELEASE,
    INPUT_EVENT_ENCODER_CW,
    INPUT_EVENT_ENCODER_CCW
} input_event_type_t;

typedef struct {
    input_event_type_t type;
    union {
        struct {
            uint8_t row;
            uint8_t col;
        } matrix;
        struct {
            uint8_t encoder_id; // 1 or 2
        } encoder;
    } data;
} input_event_t;

/**
 * @brief Initialize the input handler (matrix scanning and encoder tracking).
 * 
 * @param app_queue The queue to which input events will be posted.
 *                  Must be created by the application prior to calling this.
 */
void input_handler_init(QueueHandle_t app_queue);

#ifdef __cplusplus
}
#endif

#endif // INPUT_HANDLER_H
