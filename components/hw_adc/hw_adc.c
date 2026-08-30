#include "hw_adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/adc_types.h"

hal_adc_t *init(adc_unit_t unit_id, adc_channel_t channel_id) {
  static hal_adc_t my_handler;
  static hal_adc_config_t my_handler_config;
  static adc_oneshot_unit_handle_t handler;

  my_handler.handlers->oneshot = handler;
  my_handler.configs = &my_handler_config;
  my_handler.configs->channel = channel_id;
  my_handler.configs->unit = unit_id;

  config_adc_oneshot(&my_handler);
  config_adc_channel(&my_handler);
  config_adc_calibration(&my_handler);

  return &my_handler;
}

esp_err_t config_adc_oneshot(hal_adc_t *my_handler) {

  const adc_oneshot_unit_init_cfg_t my_config = {
      .unit_id = my_handler->configs->unit,
      .clk_src = 0,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  my_handler->configs->adc_init_cfg = my_config;

  esp_err_t ans =
      adc_oneshot_new_unit(&my_config, &(my_handler->handlers->oneshot));

  if (ans != ESP_OK && ans != ESP_ERR_INVALID_ARG) {
    // Paniquea al fallar ambos
    ESP_ERROR_CHECK(ans);
  }
  if (ans == ESP_ERR_INVALID_ARG) {
    ESP_LOGW(
        TAG,
        "Fallo al inicializar Oneshot c/cfg establecida, se usara cfg default",
        "\nUnit 1 | Gpio 2 && Channel 1");
    const adc_oneshot_unit_init_cfg_t my_default = {
        .unit_id = ADC_UNIT_1,
        .clk_src = 0,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    my_handler->configs->adc_init_cfg = my_default;
  }
  return ESP_OK;
}

esp_err_t config_adc_channel(hal_adc_t *my_handler) {
  const adc_oneshot_chan_cfg_t my_config = {.bitwidth = ADC_BITWIDTH_12,
                                            .atten = ADC_ATTEN_DB_12};
  my_handler->configs->adc_chan_cfg = my_config;

  esp_err_t ans = adc_oneshot_config_channel(
      my_handler->handlers->oneshot, my_handler->configs->channel, &my_config);
  // Paniquea al fallar config
  ESP_ERROR_CHECK(ans);

  return ESP_OK;
}

esp_err_t config_adc_calibration(hal_adc_t *my_handler) {
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = my_handler->configs->unit,
      .chan = my_handler->configs->channel,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };

  esp_err_t ans = adc_cali_create_scheme_curve_fitting(
      &cali_config, &my_handler->handlers->calib);

  if (ans == ESP_OK) {
    ESP_LOGI(
        TAG,
        "Calibración por hardware (Curve Fitting) vinculada exitosamente.");
  } else {
    ESP_LOGW(TAG, "Fallo al inicializar calibración de fábrica. Se usará "
                  "respaldo matemático.");
    my_handler->handlers->calib = NULL;
  }
  return ans;
}

void readRaw12Bit(hal_adc_t *handler, int *bit_out) {

  esp_err_t ans = adc_oneshot_read(handler->handlers->oneshot,
                                   handler->configs->channel, bit_out);
  ESP_ERROR_CHECK(ans);
  ESP_LOGI(TAG, "Raw: %4d", *(bit_out)); // Linea activa durante debug

  // int voltage_mv = 0;
  //  // Si handler de calibración listo, calcula mV
  //  if (handler->config->cali_handler != NULL) {
  //    adc_cali_raw_to_voltage(handler->config->cali_handler, adc_raw_value,
  //                            &voltage_mv);
  //  } else {
  //    voltage_mv = (adc_raw_value * 3300) / 40950;
  //  }

  // ESP_LOGI(TAG, "Raw: %4d | Voltaje: %.2f V (%d mV)", adc_raw_value,
  // voltage_mv);
}