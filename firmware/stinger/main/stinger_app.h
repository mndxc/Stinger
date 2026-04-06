#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Stinger application (Layer 3)
 * 
 * This function handles coordinating Layer 1 (Hardware Drivers) 
 * and Layer 2 (Subsystem Services) to implement the high-level 
 * soundboard behavior.
 */
void stinger_app_init(void);

#ifdef __cplusplus
}
#endif
