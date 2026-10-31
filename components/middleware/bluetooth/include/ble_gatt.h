#ifndef BLE_GATT_H
#define BLE_GATT_H

#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "esp_err.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/gap/ble_svc_gap.h"

void ble_gatt_svr_subscribe_hid_cb(struct ble_gap_event*);
int gatt_svc_init(void);
void ble_hid_set_connection(uint16_t conn_handle);
esp_err_t ble_hid_notify_latest(void);

#endif
