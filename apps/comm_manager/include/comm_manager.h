#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize communication channels (BLE HID, etc.).
 */
esp_err_t comm_manager_init(void);

/**
 * @brief Read latest bike state from data_manager, format host reports, and dispatch.
 *        Called periodically in the superloop.
 */
esp_err_t comm_manager_tick(void);

#ifdef __cplusplus
}
#endif

#endif // COMM_MANAGER_H
