#ifndef GENERIC_DATA_H
#define GENERIC_DATA_H

/**
 * @file generic-data.h
 * @brief Middleware interface for grouping sensor data.
 *
 * Provides initialization, data update, and data retrieval interfaces
 * for the aggregated sensor data.
 */

#include <stdint.h>

/**
 * @struct generic_data_t
 * @brief Structure containing the aggregated sensor data.
 *
 * Stores the angle and speed values collected from the sensors.
 */
typedef struct {
  float degangle; // Rango de angulos: -90.0 a 90.0 por lo tanto son 180 grados
  int16_t speedy; // Rango de velocidad : -255.0 a 255.0
} generic_data_t;

/**
 * @brief Initializes the aggregated sensor data.
 *
 * Sets all fields of the internal data structure to their default values.
 */
void new_static_generic(void);
/**
 * @brief Updates the speed value in the aggregated sensor data.
 *
 * @param speed New speed value to be stored.
 */
void set_speedy_data(int16_t);
/**
 * @brief Updates the angle value in the aggregated sensor data.
 *
 * @param angle New angle value in degrees.
 */
void set_degangle_data(float);
/**
 * @brief Retrieves the current aggregated sensor data.
 *
 * Returns a copy of the internal sensor data structure.
 *
 * @return Copy of the current aggregated sensor data.
 */
generic_data_t get_generic_data();

#endif
