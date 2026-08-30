#include "hw_adc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/adc_types.h"

hal_adc_t *init(adc_unit_t unit_id, adc_channel_t channel_id) {
  static hal_adc_t my_handler;
  static hal_adc_config_t my_handler_config;
  static adc_oneshot_unit_handle_t handler;

  my_handler.config = &my_handler_config;
  my_handler.config->channel = channel_id;
  my_handler.config->unit = unit_id;
  my_handler.config->adc_handler = handler;

  config_adc_oneshot(my_handler.config);
  config_adc_channel(my_handler.config);
  config_adc_calibration(my_handler.config);

  return &my_handler;
}

esp_err_t config_adc_oneshot(hal_adc_config_t *config) {

  const adc_oneshot_unit_init_cfg_t my_config = {
      .unit_id = config->unit,
      .clk_src = 0,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  config->adc_init_cfg = my_config;

  esp_err_t ans = adc_oneshot_new_unit(&my_config, &config->adc_handler);
  ESP_ERROR_CHECK(ans);
  return ESP_OK;
}

esp_err_t config_adc_channel(hal_adc_config_t *config) {
  const adc_oneshot_chan_cfg_t my_config = {.bitwidth = ADC_BITWIDTH_12,
                                            .atten = ADC_ATTEN_DB_12};
  config->adc_chan_cfg = my_config;

  esp_err_t ans = adc_oneshot_config_channel(config->adc_handler,
                                             config->channel, &my_config);
  ESP_ERROR_CHECK(ans);
  return ESP_OK;
}

esp_err_t config_adc_calibration(hal_adc_config_t *config) {
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = config->unit,
      .chan = config->channel,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };

  esp_err_t ans =
      adc_cali_create_scheme_curve_fitting(&cali_config, &config->cali_handler);

  if (ans == ESP_OK) {
    ESP_LOGI(
        TAG,
        "Calibración por hardware (Curve Fitting) vinculada exitosamente.");
  } else {
    ESP_LOGW(TAG, "Fallo al inicializar calibración de fábrica. Se usará "
                  "respaldo matemático.");
    config->cali_handler = NULL; // Asegura que quede en NULL si falla
  }
  return ans;
}

void readRaw12Bit(hal_adc_t *handler) {
  int adc_raw_value = 0;
  int voltage_mv = 0;

  esp_err_t ans = adc_oneshot_read(handler->config->adc_handler,
                                   handler->config->channel, &adc_raw_value);
  ESP_ERROR_CHECK(ans);

  // Si el handler de calibración está listo, calcula los milivoltios reales
  if (handler->config->cali_handler != NULL) {
    adc_cali_raw_to_voltage(handler->config->cali_handler, adc_raw_value,
                            &voltage_mv);
  } else {
    voltage_mv = (adc_raw_value * 3300) / 4095;
  }

  ESP_LOGI(TAG, "Raw: %4d | Voltaje: %.2f V (%d mV)", adc_raw_value,
           (float)voltage_mv / 1000.0, voltage_mv);
}