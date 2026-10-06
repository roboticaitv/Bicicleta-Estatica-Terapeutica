#ifndef BLE_GAP_H
#define BLE_GAP_H
#include "esp_err.h"
#include <stdbool.h>

esp_err_t ble_gap_init(void);
esp_err_t ble_gap_start_advertising(void);
esp_err_t ble_gap_stop_advertising(void);
bool ble_gap_is_advertising(void);
bool ble_gap_is_connected(void);

#endif