#include "ble_manager.h"
#include "ble_gap.h"
#include "ble_gatt.h"
#include "common.h"
#include "esp_log.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_store.h"

void ble_store_config_init(void);

static void host_task(void *param);
static void on_sync(void);
static void on_reset(int reason);

static void host_task(void *param) {
  (void)param;
  ESP_LOGI(TAG, "freertos Nimble host task initialized");
  nimble_port_run();
}

static void on_sync(void) {
  ESP_LOGI(TAG, "controller && host synced");
  ble_gap_init_advertising();
}

static void on_reset(int reason) {

  ESP_LOGW(TAG, "NimBLE stack reset, reason=%d", reason);
}

void ble_host_config_init(void) {

  ble_hs_cfg.sync_cb = on_sync;
  ble_hs_cfg.reset_cb = on_reset;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
  ble_hs_cfg.sm_bonding = 1;
  ble_hs_cfg.sm_sc = 1;
  ble_hs_cfg.sm_mitm = 0;
  ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
  ble_hs_cfg.sm_our_key_dist =
      BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
  ble_hs_cfg.sm_their_key_dist =
      BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
  ble_store_config_init();
}

esp_err_t ble_manager_init(void) {
  int res = 0;
  res = nvs_flash_init();
  if (res == ESP_ERR_NVS_NO_FREE_PAGES ||
      res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    res = nvs_flash_init();
  }
  if (res != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize the non volatile storage");
    return res;
  }

  res = nimble_port_init();
  if (res != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize the nimble stack, error code %d ", res);
    return res;
  }

  ble_host_config_init();

  res = gatt_svc_init();
  if (res != 0) {
    ESP_LOGE(TAG, "Failed to register HID GATT service, error code %d", res);
    return ESP_FAIL;
  }

  nimble_port_freertos_init(host_task);

  return ESP_OK;
}
