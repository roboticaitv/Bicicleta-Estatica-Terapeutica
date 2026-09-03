/**
 * @file hw_adc.c
 * @brief Implementation of the Hardware Abstraction Layer (HAL) for ADC.
 */

#include "hw_adc.h"
#include "esp_log.h"
#include "hal/adc_types.h"

static const char *TAG = "HW-ADC";
static esp_err_t config_adc_calibration(hal_adc_t *handle);
static esp_err_t config_adc_channel(hal_adc_t *handle);
static esp_err_t config_adc_oneshot(hal_adc_t *handle);

static hal_adc_t s_adc_handle;

hal_adc_t *hw_adc_init(adc_unit_t unit_id, adc_channel_t channel_id) {
  s_adc_handle.configs.channel = channel_id;
  s_adc_handle.configs.unit = unit_id;
  s_adc_handle.calibrated = false;

  if (config_adc_oneshot(&s_adc_handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize ADC oneshot unit");
    return NULL;
  }

  if (config_adc_channel(&s_adc_handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure ADC channel");
    return NULL;
  }

  config_adc_calibration(&s_adc_handle);

  return &s_adc_handle;
}

/**
 * @brief Configures the ADC oneshot unit.
 *
 * @param[in,out] handle Pointer to the HAL ADC context.
 * @return ESP_OK on success, or an error code from ESP-IDF.
 */
static esp_err_t config_adc_oneshot(hal_adc_t *my_handler) {
  if (!my_handler) {
    return ESP_ERR_INVALID_ARG;
  }

  const adc_oneshot_unit_init_cfg_t my_config = {
      .unit_id = my_handler->configs.unit,
      .clk_src = 0,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  my_handler->configs.adc_init_cfg = my_config;

  esp_err_t ans =
      adc_oneshot_new_unit(&my_config, &(my_handler->handlers.oneshot));
  if (ans != ESP_OK && ans != ESP_ERR_INVALID_ARG) {
    ESP_ERROR_CHECK(ans);
  }
  if (ans == ESP_ERR_INVALID_ARG) {
    ESP_LOGW(TAG, "Failed to initialize Oneshot with specified cfg, default cfg "
                  "will be used (Unit 1)");
    const adc_oneshot_unit_init_cfg_t my_default = {
        .unit_id = ADC_UNIT_1,
        .clk_src = 0,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    my_handler->configs.adc_init_cfg = my_default;
    ans = adc_oneshot_new_unit(&my_default, &(my_handler->handlers.oneshot));
  }
  return ans;
}

/**
 * @brief Configures the specific ADC channel bitwidth and attenuation.
 *
 * @param[in,out] handle Pointer to the HAL ADC context.
 * @return ESP_OK on success, or an error code from ESP-IDF.
 */
static esp_err_t config_adc_channel(hal_adc_t *my_handler) {
  if (!my_handler) {
    return ESP_ERR_INVALID_ARG;
  }

  const adc_oneshot_chan_cfg_t my_config = {.bitwidth = ADC_BITWIDTH_12,
                                            .atten = ADC_ATTEN_DB_12};
  my_handler->configs.adc_chan_cfg = my_config;

  esp_err_t ans = adc_oneshot_config_channel(
      my_handler->handlers.oneshot, my_handler->configs.channel, &my_config);
  ESP_ERROR_CHECK(ans);

  return ans;
}

/**
 * @brief Attempts to initialize hardware calibration (Line or Curve Fitting
 * scheme).
 *
 * @param[in,out] handle Pointer to the HAL ADC context.
 * @return ESP_OK on success, or an error code if calibration scheme is
 * unsupported or failed.
 */

static esp_err_t config_adc_calibration(hal_adc_t *my_handler) {
  if (!my_handler) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ans = ESP_ERR_NOT_SUPPORTED;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = my_handler->configs.unit,
      .chan = my_handler->configs.channel,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ans = adc_cali_create_scheme_curve_fitting(&cali_config,
                                             &my_handler->handlers.calib);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = my_handler->configs.unit,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ans = adc_cali_create_scheme_line_fitting(&cali_config,
                                            &my_handler->handlers.calib);
#endif

  if (ans == ESP_OK) {
    ESP_LOGI(TAG, "Hardware ADC calibration successfully configured.");
    my_handler->calibrated = true;
  } else {
    ESP_LOGW(
        TAG,
        "Factory ADC calibration not available. Using uncalibrated fallback.");
    my_handler->handlers.calib = NULL;
    my_handler->calibrated = false;
  }
  return ans;
}

void readRaw12Bit(hal_adc_t *handler, int *bit_out) {
  if (!handler || !bit_out) {
    return;
  }
  esp_err_t ans = adc_oneshot_read(handler->handlers.oneshot,
                                   handler->configs.channel, bit_out);
  ESP_ERROR_CHECK(ans);
  ESP_LOGD(TAG, "ADC Raw: %4d", *bit_out);
}

esp_err_t hw_adc_read_voltage(hal_adc_t *handle, int *voltage_mv) {
  if (!handle || !voltage_mv) {
    return ESP_ERR_INVALID_ARG;
  }

  int raw = 0;
  readRaw12Bit(handle, &raw);

  if (handle->calibrated && handle->handlers.calib != NULL) {
    return adc_cali_raw_to_voltage(handle->handlers.calib, raw, voltage_mv);
  } else {
    // Fallback calculation for 12-bit (0..4095) with 12dB attenuation (~3300mV
    // max)
    *voltage_mv = (raw * 3300) / 4095;
    return ESP_OK;
  }
}
