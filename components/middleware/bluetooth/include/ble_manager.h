#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H
#include "esp_err.h"
#include <stdbool.h>
#include <stdio.h>

esp_err_t ble_manager_init(void);
esp_err_t ble_manager_start_advertising(void);
esp_err_t ble_manager_stop_advertising(void);
bool ble_manager_is_connected(void);
bool ble_manager_is_advertising(void);

#endif