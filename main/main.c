#include "ble_manager.h"
#include "ble_gatt.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "generic-data.h"
#include "steering_wheel.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
  ESP_LOGI(TAG, "Biciver initialization");

  ESP_ERROR_CHECK(generic_data_init());
  ESP_ERROR_CHECK(steering_wheel_init());
  ESP_ERROR_CHECK(ble_manager_init());

  while (true) {
    steering_wheel_sample_t sample;
    if (steering_wheel_sample(&sample) == ESP_OK) {
      esp_err_t err = generic_data_set_float(
          GENERIC_SIGNAL_STEERING_ANGLE, sample.angle_degrees);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to publish steering signal: %s",
                 esp_err_to_name(err));
      }
    }
    esp_err_t notify_err = ble_hid_notify_latest();
    if (notify_err != ESP_OK && notify_err != ESP_ERR_INVALID_STATE) {
      ESP_LOGW(TAG, "HID report notification failed: %s",
               esp_err_to_name(notify_err));
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
