#include "data_manager.h"

#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "steering_wheel.h"

static const char *TAG = "DATA_MGR";

#define STEERING_DEADZONE_DEG 0.5f  // Suppress ADC noise near center
#define STEERING_MAX_DEG      90.0f

static bike_state_t s_current_state = {0};
static portMUX_TYPE s_state_mux = portMUX_INITIALIZER_UNLOCKED;

esp_err_t data_manager_init(void) {
  taskENTER_CRITICAL(&s_state_mux);
  s_current_state.steering_angle_degrees = 0.0f;
  s_current_state.pedal_rpm = 0;
  taskEXIT_CRITICAL(&s_state_mux);
  ESP_LOGI(TAG, "Data manager initialized");
  return ESP_OK;
}

esp_err_t data_manager_tick(void) {
  steering_wheel_sample_t sample;
  float filtered_angle = 0.0f;

  if (steering_wheel_sample(&sample) == ESP_OK) {
    float angle = sample.angle_degrees;
    if (isfinite(angle)) {
      // 1. Center deadband to suppress potentiometer/ADC jitter
      if (fabsf(angle) < STEERING_DEADZONE_DEG) {
        angle = 0.0f;
      }
      // 2. Physical clamping
      if (angle < -STEERING_MAX_DEG) angle = -STEERING_MAX_DEG;
      if (angle >  STEERING_MAX_DEG) angle =  STEERING_MAX_DEG;
      filtered_angle = angle;
    }
  }

  // Deposit clean state into mailbox
  taskENTER_CRITICAL(&s_state_mux);
  s_current_state.steering_angle_degrees = filtered_angle;
  s_current_state.pedal_rpm = 0; // Placeholder until cadence sensor is added
  taskEXIT_CRITICAL(&s_state_mux);

  return ESP_OK;
}

esp_err_t data_manager_get_state(bike_state_t *out_state) {
  if (out_state == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  taskENTER_CRITICAL(&s_state_mux);
  *out_state = s_current_state;
  taskEXIT_CRITICAL(&s_state_mux);
  return ESP_OK;
}
