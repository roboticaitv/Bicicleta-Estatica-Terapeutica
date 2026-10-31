#include "ble_gap.h"

#include "ble_gatt.h"
#include "common.h"
#include "host/ble_gap.h"

static uint8_t own_addr_type;
static const ble_uuid16_t hid_service_uuid = BLE_UUID16_INIT(0x1812);
static bool s_connected;

static void start_advertising(void);
static int gap_event_handler(struct ble_gap_event *event, void *arg);

static void start_advertising(void) {
  struct ble_hs_adv_fields adv_fields = {0};
  struct ble_hs_adv_fields rsp_fields = {0};
  struct ble_gap_adv_params adv_params = {0};
  int rc;

  static const char short_name[] = "BICIVER-HID";
  adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  adv_fields.name = (uint8_t *)short_name;
  adv_fields.name_len = sizeof(short_name) - 1;
  adv_fields.name_is_complete = 0;
  adv_fields.uuids16 = (ble_uuid16_t *)&hid_service_uuid;
  adv_fields.num_uuids16 = 1;
  adv_fields.uuids16_is_complete = 1;
  adv_fields.appearance = 0x03C4; /* Gamepad */
  adv_fields.appearance_is_present = 1;
  adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
  adv_fields.tx_pwr_lvl_is_present = 1;

  rc = ble_gap_adv_set_fields(&adv_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to set advertising data: %d", rc);
    return;
  }

  rsp_fields.name = (uint8_t *)BLE_DEVICE_NAME;
  rsp_fields.name_len = strlen(BLE_DEVICE_NAME);
  rsp_fields.name_is_complete = 1;
  rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to set scan response data: %d", rc);
    return;
  }

  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                         gap_event_handler, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to start advertising: %d", rc);
    return;
  }
  ESP_LOGI(TAG, "Advertising HID gamepad as %s", BLE_DEVICE_NAME);
}

static int gap_event_handler(struct ble_gap_event *event, void *arg) {
  (void)arg;
  switch (event->type) {
  case BLE_GAP_EVENT_CONNECT:
    ESP_LOGI(TAG, "Connection %s; status=%d",
             event->connect.status == 0 ? "established" : "failed",
             event->connect.status);
    if (event->connect.status == 0) {
      s_connected = true;
      ble_hid_set_connection(event->connect.conn_handle);
      int security_rc = ble_gap_security_initiate(event->connect.conn_handle);
      if (security_rc != 0) {
        ESP_LOGW(TAG, "Could not start HID pairing: %d", security_rc);
      }
      const struct ble_gap_upd_params params = {
          .itvl_min = 12,             /* 15 ms */
          .itvl_max = 24,             /* 30 ms */
          .latency = 0,
          .supervision_timeout = 400, /* 4 s */
          .min_ce_len = 0,
          .max_ce_len = 0,
      };
      int rc = ble_gap_update_params(event->connect.conn_handle, &params);
      if (rc != 0) {
        ESP_LOGW(TAG, "Could not request HID connection interval: %d", rc);
      }
    } else {
      s_connected = false;
      ble_hid_set_connection(BLE_HS_CONN_HANDLE_NONE);
      start_advertising();
    }
    return 0;

  case BLE_GAP_EVENT_DISCONNECT:
    ESP_LOGI(TAG, "Disconnected; reason=%d", event->disconnect.reason);
    s_connected = false;
    ble_hid_set_connection(BLE_HS_CONN_HANDLE_NONE);
    start_advertising();
    return 0;

  case BLE_GAP_EVENT_ADV_COMPLETE:
    if (event->adv_complete.reason != 0) {
      ESP_LOGI(TAG, "Advertising complete; reason=%d",
               event->adv_complete.reason);
    }
    if (!s_connected) {
      start_advertising();
    }
    return 0;

  case BLE_GAP_EVENT_ENC_CHANGE:
    if (event->enc_change.status != 0) {
      ESP_LOGW(TAG, "HID link encryption failed: %d", event->enc_change.status);
      ble_gap_terminate(event->enc_change.conn_handle,
                        BLE_ERR_REM_USER_CONN_TERM);
    } else {
      ESP_LOGI(TAG, "HID link encrypted and bonded");
    }
    return 0;

  case BLE_GAP_EVENT_SUBSCRIBE:
    ble_gatt_svr_subscribe_hid_cb(event);
    return 0;

  case BLE_GAP_EVENT_MTU:
    ESP_LOGD(TAG, "MTU updated to %u", event->mtu.value);
    return 0;

  case BLE_GAP_EVENT_CONN_UPDATE:
    ESP_LOGI(TAG, "Connection parameters updated; status=%d",
             event->conn_update.status);
    return 0;

  case BLE_GAP_EVENT_NOTIFY_TX:
    if (event->notify_tx.status != 0 &&
        event->notify_tx.status != BLE_HS_EDONE) {
      ESP_LOGW(TAG, "HID notification status=%d", event->notify_tx.status);
    }
    return 0;

  default:
    return 0;
  }
}

void ble_gap_init_advertising(void) {
  int rc = ble_hs_util_ensure_addr(0);
  if (rc != 0) {
    ESP_LOGE(TAG, "No Bluetooth device address is available: %d", rc);
    return;
  }
  rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to infer Bluetooth address type: %d", rc);
    return;
  }
  start_advertising();
}
