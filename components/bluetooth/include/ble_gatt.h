#ifndef BLE_GATT_H
#define BLE_GATT_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "ble_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
  int16_t x;
  int16_t y;
} ble_hid_gamepad_report_t;

void ble_gatt_svr_subscribe_hid_cb(struct ble_gap_event *event);
int gatt_svc_init(const ble_config_t *config);
void ble_hid_set_connection(uint16_t conn_handle);
esp_err_t ble_hid_set_report(const ble_hid_gamepad_report_t *report);
esp_err_t ble_hid_notify_latest(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_GATT_H
