#include "steering_wheel.h"

#include <math.h>
#include <string.h>

#include "adc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "sdkconfig.h"

static const char *TAG = "STEERING_WHEEL";
static hal_adc_t *s_adc_handle;
static int s_filter_values[CONFIG_BICIVER_STEERING_FILTER_WINDOW];
static size_t s_filter_count;
static size_t s_filter_next;
static int s_center_raw = CONFIG_BICIVER_ADC_CENTER_RAW;

static bool gpio_configuration_valid(void) {
  const int pins[] = {CONFIG_BICIVER_ADC_GPIO,
                      CONFIG_BICIVER_ENCODER_A_GPIO,
                      CONFIG_BICIVER_ENCODER_B_GPIO,
                      CONFIG_BICIVER_LIMIT_LOW_LED_GPIO,
                      CONFIG_BICIVER_LIMIT_HIGH_LED_GPIO};
  for (size_t i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i) {
    if (!GPIO_IS_VALID_GPIO(pins[i])) {
      ESP_LOGE(TAG, "GPIO %d is not valid for this target", pins[i]);
      return false;
    }
    for (size_t j = i + 1; j < sizeof(pins) / sizeof(pins[0]); ++j) {
      if (pins[i] == pins[j]) {
        ESP_LOGE(TAG, "GPIO %d is assigned to multiple functions", pins[i]);
        return false;
      }
    }
  }
  if (CONFIG_BICIVER_ADC_MIN_RAW >= s_center_raw ||
      s_center_raw >= CONFIG_BICIVER_ADC_MAX_RAW) {
    ESP_LOGE(TAG, "ADC limits must satisfy min < center < max");
    return false;
  }
  return true;
}

esp_err_t steering_wheel_init(void) {
  if (!gpio_configuration_valid()) {
    return ESP_ERR_INVALID_ARG;
  }

  adc_unit_t adc_unit;
  adc_channel_t adc_channel;
  esp_err_t err = adc_oneshot_io_to_channel(CONFIG_BICIVER_ADC_GPIO,
                                            &adc_unit, &adc_channel);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "GPIO %d cannot be used as an ADC input: %s",
             CONFIG_BICIVER_ADC_GPIO, esp_err_to_name(err));
    return err;
  }
  s_adc_handle = hw_adc_init(adc_unit, adc_channel);
  if (s_adc_handle == NULL) {
    return ESP_FAIL;
  }

  gpio_config_t led_config = {
      .pin_bit_mask = (1ULL << CONFIG_BICIVER_LIMIT_LOW_LED_GPIO) |
                      (1ULL << CONFIG_BICIVER_LIMIT_HIGH_LED_GPIO),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  err = gpio_config(&led_config);
  if (err != ESP_OK) {
    return err;
  }
#if CONFIG_BICIVER_LIMIT_LEDS_ACTIVE_LOW
  gpio_set_level(CONFIG_BICIVER_LIMIT_LOW_LED_GPIO, 1);
  gpio_set_level(CONFIG_BICIVER_LIMIT_HIGH_LED_GPIO, 1);
#else
  gpio_set_level(CONFIG_BICIVER_LIMIT_LOW_LED_GPIO, 0);
  gpio_set_level(CONFIG_BICIVER_LIMIT_HIGH_LED_GPIO, 0);
#endif
  memset(s_filter_values, 0, sizeof(s_filter_values));
  s_filter_count = 0;
  s_filter_next = 0;
  ESP_LOGI(TAG, "ADC GPIO %d; limits %d/%d/%d; moving average window %d",
           CONFIG_BICIVER_ADC_GPIO, CONFIG_BICIVER_ADC_MIN_RAW, s_center_raw,
           CONFIG_BICIVER_ADC_MAX_RAW, CONFIG_BICIVER_STEERING_FILTER_WINDOW);
  return ESP_OK;
}

static int filter_adc_sample(int raw) {
  s_filter_values[s_filter_next] = raw;
  s_filter_next = (s_filter_next + 1) % CONFIG_BICIVER_STEERING_FILTER_WINDOW;
  if (s_filter_count < CONFIG_BICIVER_STEERING_FILTER_WINDOW) {
    ++s_filter_count;
  }
  int64_t total = 0;
  for (size_t i = 0; i < s_filter_count; ++i) {
    total += s_filter_values[i];
  }
  return (int)(total / (int64_t)s_filter_count);
}

esp_err_t steering_wheel_sample(steering_wheel_sample_t *sample) {
  if (sample == NULL || s_adc_handle == NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  int raw = 0;
  readRaw12Bit(s_adc_handle, &raw);
  const int filtered = filter_adc_sample(raw);
  const bool below = filtered < CONFIG_BICIVER_ADC_MIN_RAW;
  const bool above = filtered > CONFIG_BICIVER_ADC_MAX_RAW;
  const int clamped = filtered < CONFIG_BICIVER_ADC_MIN_RAW
                          ? CONFIG_BICIVER_ADC_MIN_RAW
                          : (filtered > CONFIG_BICIVER_ADC_MAX_RAW
                                 ? CONFIG_BICIVER_ADC_MAX_RAW
                                 : filtered);

  float angle;
  if (clamped <= s_center_raw) {
    angle = -(float)(s_center_raw - clamped) *
            CONFIG_BICIVER_STEERING_MAX_ANGLE_DEG /
            (float)(s_center_raw - CONFIG_BICIVER_ADC_MIN_RAW);
  } else {
    angle = (float)(clamped - s_center_raw) *
            CONFIG_BICIVER_STEERING_MAX_ANGLE_DEG /
            (float)(CONFIG_BICIVER_ADC_MAX_RAW - s_center_raw);
  }

#if CONFIG_BICIVER_LIMIT_LEDS_ACTIVE_LOW
  gpio_set_level(CONFIG_BICIVER_LIMIT_LOW_LED_GPIO, below ? 0 : 1);
  gpio_set_level(CONFIG_BICIVER_LIMIT_HIGH_LED_GPIO, above ? 0 : 1);
#else
  gpio_set_level(CONFIG_BICIVER_LIMIT_LOW_LED_GPIO, below ? 1 : 0);
  gpio_set_level(CONFIG_BICIVER_LIMIT_HIGH_LED_GPIO, above ? 1 : 0);
#endif
  *sample = (steering_wheel_sample_t){
      .adc_raw = filtered,
      .angle_degrees = angle,
      .below_lower_limit = below,
      .above_upper_limit = above,
  };
  return ESP_OK;
}

esp_err_t steering_wheel_calibrate_center(void) {
  if (s_adc_handle == NULL) {
    return ESP_ERR_INVALID_STATE;
  }
  int raw = 0;
  readRaw12Bit(s_adc_handle, &raw);
  if (raw <= CONFIG_BICIVER_ADC_MIN_RAW ||
      raw >= CONFIG_BICIVER_ADC_MAX_RAW) {
    return ESP_ERR_INVALID_ARG;
  }
  s_center_raw = raw;
  ESP_LOGI(TAG, "Steering center calibrated to %d", s_center_raw);
  return ESP_OK;
}
