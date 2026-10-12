#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H
#include "esp_err.h"
#include <stdbool.h>
#include <stdio.h>

esp_err_t ble_manager_init(void);
void ble_host_config_init(void);

#endif