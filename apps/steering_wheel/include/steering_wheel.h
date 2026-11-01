#ifndef STEERING_WHEEL_H
#define STEERING_WHEEL_H

#include <stdbool.h>

#include "esp_err.h"

typedef struct {
  int adc_raw;
  float angle_degrees;
  bool below_lower_limit;
  bool above_upper_limit;
} steering_wheel_sample_t;

esp_err_t steering_wheel_init(void);
esp_err_t steering_wheel_sample(steering_wheel_sample_t *sample);
esp_err_t steering_wheel_calibrate_center(void);

#endif
