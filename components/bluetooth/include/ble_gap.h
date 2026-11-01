#ifndef BLE_GAP_H
#define BLE_GAP_H

#include "ble_manager.h"
#include "esp_err.h"

void ble_gap_init_advertising(const ble_config_t *config);

#endif // BLE_GAP_H
