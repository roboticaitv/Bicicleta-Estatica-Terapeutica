#include "ble_gap.h"

typedef struct {
  // Valores despues de encontrarle
} ble_gap_configs_t;

typedef struct {
  bool is_advertising;
  bool is_connected;
} ble_gap_states_t;

typedef struct {
  ble_gap_states_t ble_states;
  ble_gap_configs_t ble_configs;
} ble_gap_t;

static ble_gap_t gap_manager;

esp_err_t ble_gap_init(void) { return ESP_OK; };
esp_err_t ble_gap_start_advertising(void) { return ESP_OK; };
esp_err_t ble_gap_stop_advertising(void) { return ESP_OK; };
bool ble_gap_is_advertising(void){
    return gap_manager.ble_states.is_advertising;    
}
bool ble_gap_is_connected(void){
    return gap_manager.ble_states.is_connected;
}
