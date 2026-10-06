#include "ble_manager.h"
#include "ble_gap.h"

esp_err_t ble_manager_init(void) {
  esp_err_t gap_result = ble_gap_init();
  if (gap_result != ESP_OK) {
    return gap_result;
  }

  return ESP_OK;
}
bool ble_manager_is_connected(void) { return ble_gap_is_connected(); }
bool ble_manager_is_advertising(void) { return ble_gap_is_advertising(); }
esp_err_t ble_manager_start_advertising(void) {
  return ble_gap_start_advertising();
}
esp_err_t ble_manager_stop_advertising(void) {
  return ble_gap_stop_advertising();
}
