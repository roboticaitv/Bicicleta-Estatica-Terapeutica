#include "ble_gap.h"
#include "common.h"

#include "esp_log.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "host/ble_hs_adv.h"
#include "host/ble_hs_id.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint8_t own_addr_type;
static uint8_t addr_val[6] = {0};
static uint8_t esp_uri[] = {0x17, '/', '/', 'r', 'o', 'b', 'o', 't',
                            'i',  'c', 'a', '.', 'c', 'l', 'u', 'b'};

esp_err_t ble_gap_config_init(void) {
  int rc = 0;

  ble_svc_gap_init();

  rc = ble_svc_gap_device_name_set(DEVICE_NAME);
  if (rc != ESP_OK) {
    ESP_LOGE(TAG, "couldn't set a device name for the device, error code: %d",
             rc);
  }

  rc = ble_svc_gap_device_appearance_set(0x03C0);

  return rc;
};

static void ble_gap_start_advertising() {
  
  int rc = 0;
  const char *name = DEVICE_NAME;
  struct ble_hs_adv_fields adv_fields = {0};
  struct ble_hs_adv_fields rsp_fields = {0};
  struct ble_gap_adv_params adv_params = {0};

  adv_fields.name = (uint8_t *)name;
  adv_fields.name_len = strlen(name);
  adv_fields.name_is_complete = 1;

  adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
  adv_fields.tx_pwr_lvl_is_present = 1;

  adv_fields.appearance = 0x03c0;
  adv_fields.appearance_is_present = 1;

  rc = ble_gap_adv_set_fields(&adv_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set advertising data, error code: %d", rc);
    return;
  }

  rsp_fields.device_addr = addr_val;
  rsp_fields.device_addr_type = own_addr_type;
  rsp_fields.device_addr_is_present = 1;

  rsp_fields.uri = esp_uri;
  rsp_fields.uri_len = sizeof(esp_uri);

  rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
  if (rc != 0) {
    ESP_LOGE(TAG, "failed to set scan response data, error code: %d", rc);
    return;
  }

  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  
  rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to start advertising error code: %d", rc);
    return;
  }
  ESP_LOGI(TAG, "Advertising started");
}

void ble_gap_init_advertising(void) {

  int rc = 0;
  rc = ble_hs_util_ensure_addr(0);
  if (rc != 0) {
    ESP_LOGE(TAG, "Device doesn't have any available bt addres");
    return;
  }

  rc = ble_hs_id_infer_auto(0, &own_addr_type);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to infer address type, error code: %d", rc);
    return;
  }

  rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);
  if (rc != 0) {
    ESP_LOGI(TAG, "Failed to copy device address, error code: %d", rc);
    return;
  }
  ble_gap_start_advertising();
};

