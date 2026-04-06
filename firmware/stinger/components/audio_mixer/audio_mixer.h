#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include "esp_err.h"

/**
 * @brief Initialize the audio mixer subsystem
 * @return esp_err_t ESP_OK on success
 */
esp_err_t audio_mixer_init(void);

#endif // AUDIO_MIXER_H
