#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gatt/ble_svc_gatt.h"

void ble_gatt_svr_subscribe_hid_cb(struct ble_gap_event*);
void send_bicycle_event_indication(void);