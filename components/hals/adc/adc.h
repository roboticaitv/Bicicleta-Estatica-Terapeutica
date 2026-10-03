#ifndef HW_ADC_H
#define HW_ADC_H

/**
 * @file hw_adc.h
 * @brief Hardware Abstraction Layer (HAL) for ESP32 Analog-to-Digital Converter
 * (ADC).
 *
 * Provides configuration, initialization, calibration, and reading interfaces
 * for ADC channels using the ESP-IDF ADC oneshot driver.
 */

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "hal/adc_types.h"
#include <stdbool.h>

/**
 * @brief Configuration structure for the HAL ADC unit and channel.
 */
typedef struct {
  adc_unit_t unit;       /**< ADC unit ID (e.g. ADC_UNIT_1) */
  adc_channel_t channel; /**< ADC channel ID */
  adc_oneshot_unit_init_cfg_t
      adc_init_cfg; /**< One-shot unit initialization configuration */
  adc_oneshot_chan_cfg_t adc_chan_cfg; /**< One-shot channel configuration */
} hal_adc_config_t;

/**
 * @brief Handlers for ADC driver instances and calibration schemes.
 */
typedef struct {
  adc_oneshot_unit_handle_t oneshot; /**< ESP-IDF ADC oneshot unit handle */
  adc_cali_handle_t calib;           /**< ESP-IDF ADC calibration handle */
} hal_adc_handlers_t;

/**
 * @brief Top-level HAL ADC driver handle context.
 */
typedef struct {
  hal_adc_config_t configs;    /**< ADC configurations */
  hal_adc_handlers_t handlers; /**< ADC handles */
  bool calibrated;             /**< Calibration status flag */
} hal_adc_t;

/**
 * @brief Initializes and configures an ADC channel with 12-bit resolution and
 * 12dB attenuation.
 *
 * @param[in] unit_id    The ADC unit (e.g., ADC_UNIT_1).
 * @param[in] channel_id The ADC channel (e.g., ADC_CHANNEL_3).
 * @return Pointer to the initialized hal_adc_t handle, or NULL on failure.
 */
hal_adc_t *hw_adc_init(adc_unit_t unit_id, adc_channel_t channel_id);

    /**
     * @brief Reads the raw 12-bit ADC value.
     *
     * @param[in]  handle  Pointer to the HAL ADC context.
     * @param[out] bit_out Pointer to integer where the raw reading will be
     * stored.
     */
    void readRaw12Bit(hal_adc_t *handle, int *bit_out);

/**
 * @brief Reads calibrated voltage in millivolts.
 *
 * @param[in]  handle     Pointer to the HAL ADC context.
 * @param[out] voltage_mv Pointer to integer where the millivolts will be
 * stored.
 * @return ESP_OK on success, or error code.
 */
esp_err_t hw_adc_read_voltage(hal_adc_t *handle, int *voltage_mv);

#endif