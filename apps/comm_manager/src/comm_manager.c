#include "comm_manager.h"

#include <math.h>
#include "esp_log.h"
#include "data_manager.h"
#include "ble_gatt.h"

static const char *TAG = "COMM_MGR";

static int16_t s_last_sent_x = 0;
static int16_t s_last_sent_y = 0;
static bool s_first_report = true;

esp_err_t comm_manager_init(void) {
  s_last_sent_x = 0;
  s_last_sent_y = 0;
  s_first_report = true;
  ESP_LOGI(TAG, "Communication manager initialized");
  return ESP_OK;
}

esp_err_t comm_manager_tick(void) {
  bike_state_t state;
  if (data_manager_get_state(&state) != ESP_OK) {
    return ESP_FAIL;
  }

  // Map physical domain units to BLE HID Gamepad Report (-90..90, -512..512)
  int16_t axis_x = (int16_t)lroundf(state.steering_angle_degrees);
  int16_t axis_y = (int16_t)state.pedal_rpm;

  // Only dispatch if data changed or first report after startup
  if (s_first_report || axis_x != s_last_sent_x || axis_y != s_last_sent_y) {
    s_first_report = false;
    s_last_sent_x = axis_x;
    s_last_sent_y = axis_y;

    ble_hid_gamepad_report_t report = {
      .x = axis_x,
      .y = axis_y,
    };

    esp_err_t err = ble_hid_set_report(&report);
    if (err != ESP_OK) {
      return err;
    }

    esp_err_t notify_err = ble_hid_notify_latest();
    if (notify_err != ESP_OK && notify_err != ESP_ERR_INVALID_STATE) {
      ESP_LOGW(TAG, "HID notify failed: %s", esp_err_to_name(notify_err));
    }
  }

  return ESP_OK;
}
