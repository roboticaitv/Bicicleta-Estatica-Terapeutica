#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include <stdint.h>

// Usar unidad 1 (preferiblemente)

static const char *TAG = "HW-ADC";

typedef struct {
  adc_unit_t unit;
  adc_channel_t channel;
  adc_oneshot_unit_handle_t adc_handler;
  adc_cali_handle_t
      cali_handler; // NUEVO: Manejador para la calibración de voltaje
  adc_oneshot_unit_init_cfg_t adc_init_cfg;
  adc_oneshot_chan_cfg_t adc_chan_cfg;
} hal_adc_config_t;

typedef struct {
  uint16_t raw_value : 12;
  hal_adc_config_t *config;
} hal_adc_t;

hal_adc_t *init(adc_unit_t, adc_channel_t);
esp_err_t config_adc_oneshot(hal_adc_config_t *);
esp_err_t config_adc_channel(hal_adc_config_t *);
esp_err_t config_adc_calibration(hal_adc_config_t *config);
void readRaw12Bit(hal_adc_t *);