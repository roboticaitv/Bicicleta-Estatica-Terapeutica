#include "generic-data.h"
#include "esp_log.h"
#include <stdint.h>

/**
 * @file generic-data.c
 * @brief Implementation of the generic sensor data middleware.
 *
 * Provides the internal storage and implementation for initializing,
 * updating, and retrieving the aggregated sensor data.
 */
static const char *TAG = "GENERIC_DATA";

static generic_data_t generic_data;

/**
 * @brief Initializes the internal aggregated sensor data.
 *
 * Sets the angle and speed values to their default values and logs
 * the initialization event.
 */
void new_static_generic() {
  generic_data.degangle = 0.0f;
  generic_data.speedy = 0.0f;
  ESP_LOGI(
      TAG,
      "Generic data application succesfully started with 0.0 && 0.0 values");
}

/**
 * @brief Updates the stored speed value.
 *
 * @param speed New speed value to be stored.
 */

void set_speedy_data(int16_t speed) {
  ESP_LOGW(TAG, "Not implemented");
  generic_data.speedy = speed;
}

/**
 * @brief Updates the stored angle value.
 *
 * @param angle New angle value in degrees.
 */
void set_degangle_data(float angle) {
  ESP_LOGI(TAG, "Generic Angle: %.2f deg", angle);
  generic_data.degangle = angle;
}

/**
 * @brief Retrieves a copy of the internal aggregated sensor data.
 *
 * The returned structure is a copy of the middleware's internal data,
 * allowing the caller to access the current values without directly
 * accessing the internal storage.
 *
 * @return Copy of the current aggregated sensor data.
 */
generic_data_t get_generic_data() { return generic_data; }