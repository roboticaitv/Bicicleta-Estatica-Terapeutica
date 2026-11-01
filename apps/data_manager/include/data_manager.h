#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float steering_angle_degrees;  // Clean, filtered angle (-90.0 .. 90.0)
  int32_t pedal_rpm;             // Clean cadence
} bike_state_t;

/**
 * @brief Initialize sensor data manager.
 */
esp_err_t data_manager_init(void);

/**
 * @brief Sample physical sensors, filter noise/deadbands, and update state.
 *        Called periodically in the superloop.
 */
esp_err_t data_manager_tick(void);

/**
 * @brief Fetch the latest filtered bike state from the mailbox.
 */
esp_err_t data_manager_get_state(bike_state_t *out_state);

#ifdef __cplusplus
}
#endif

#endif // DATA_MANAGER_H
