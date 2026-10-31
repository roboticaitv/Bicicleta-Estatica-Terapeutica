#include "ble_gatt.h"
#include <stdint.h>

static const char *TAG = "BLE_GATT";

static const ble_uuid16_t hid_svc_uuid = BLE_UUID16_INIT(0x1812);
static uint16_t hid_svc_chr_val_handler;
static uint16_t hid_svc_chr_conn_handler = 0;
// TODO: Inicializar una app generica de los datos para poder acceder y
// guardalos en el ble char

static int hid_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg) {
  int rc = 0;

  switch (ctxt->op) {
  case BLE_GATT_ACCESS_OP_WRITE_CHR:
    if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
      ESP_LOGI(TAG, "characteristic write; conn_handle=%d attr_handle=%d",
               conn_handle, attr_handle);
    } else {
      ESP_LOGI(TAG, "characteristic write by nimble stack; attr_handle=%d",
               attr_handle);
    }

    // TODO: ACCEDER A LOS DATOS CON GENERICOS Y CREAR IMPLEMENTACIONES
    break;
  case BLE_GATT_ACCESS_OP_READ_CHR:
    if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
      ESP_LOGI(TAG, "characteristic read; conn_handle=%d attr_handle=%d",
               conn_handle, attr_handle);
    } else {
      ESP_LOGI(TAG, "characteristic read by nimble stack; attr_handle=%d",
               attr_handle);
    }

    // TODO: ACCEDER A LOS DATOS CON GENERICOS Y CREAR IMPLEMENTACIONES
    break;
  default:
    goto error;
  error: {
    ESP_LOGE(
        TAG,
        "unexpected access operation to hid service characteristic, opcode: %d",
        ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
  }
  }

  return rc;
}
static const struct ble_gatt_svc_def gatt_hid_svr_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &hid_svc_uuid.u,
     .characteristics = &((struct ble_gatt_chr_def){
         .uuid = &hid_svc_uuid.u,
         .access_cb = hid_chr_access,
         .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
         .val_handle = &hid_svc_chr_val_handler,
     })},
    {0},
};

void send_bicycle_event_indication(void) {
  
}

void ble_gatt_svr_subscribe_hid_cb(struct ble_gap_event *event) {
  if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
    ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d",
             event->subscribe.conn_handle, event->subscribe.attr_handle);
  } else {
    ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d",
             event->subscribe.attr_handle);
  }

if (event->subscribe.attr_handle == hid_svc_chr_val_handler) {
  }
}

int gatt_svc_init() {
  int rc = 0;

  ble_svc_gatt_init();

  rc = ble_gatts_count_cfg(&gatt_hid_svr_svcs[0]);
  if (rc != 0) {
    return rc;
  }

  rc = ble_gatts_add_svcs(&gatt_hid_svr_svcs[0]);
  if (rc != 0) {
    return rc;
  }

  return 0;
}
