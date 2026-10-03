#ifndef STEERING_WHEEL_H
#define STEERING_WHEEL_H

/**
 * @file steering_wheel.h
 * @brief High-level application feature for bicycle steering angle acquisition and calibration.
 *
 * This feature translates low-level ADC voltage/raw values from a potentiometer or rotary
 * sensor mounted on the steering assembly into physical steering angles in degrees.
 */

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Steering wheel configuration structure.
 */
typedef struct {
    int adc_min_raw;        /**< Raw ADC reading at maximum left turn */
    int adc_max_raw;        /**< Raw ADC reading at maximum right turn */
    int adc_center_raw;     /**< Raw ADC reading at center position */
    float max_angle_deg;    /**< Maximum mechanical steering angle in degrees (e.g. +/- 45.0) */
} steering_wheel_config_t;

/**
 * @brief Initializes the steering wheel sensor interface and underlying HAL ADC channel.
 *
 * @return ESP_OK on success, or an error code from the HAL driver.
 */
esp_err_t steering_wheel_init(void);

/**
 * @brief Reads the current steering angle in degrees.
 *
 * @return Steering angle in degrees (negative for left, positive for right, 0.0 for center).
 */
float steering_wheel_get_angle_degrees(void);

/**
 * @brief Calibrates the current position as the center (0 degrees) position.
 *
 * @return ESP_OK on success, or ESP_FAIL if the sensor is uninitialized.
 */
esp_err_t steering_wheel_calibrate_center(void);

#endif /* STEERING_WHEEL_H */
