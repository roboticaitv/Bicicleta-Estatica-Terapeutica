#include "ble_gatt.h"

#include <math.h>
#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "generic-data.h"
#include "ble_config.h"
#include "host/ble_gap.h"
#include "host/ble_hs.h"
#include "os/os_mbuf.h"

static const char *TAG = "BLE_HID";

static const ble_uuid16_t hid_service_uuid = BLE_UUID16_INIT(0x1812);
static const ble_uuid16_t hid_info_uuid = BLE_UUID16_INIT(0x2A4A);
static const ble_uuid16_t report_map_uuid = BLE_UUID16_INIT(0x2A4B);
static const ble_uuid16_t report_uuid = BLE_UUID16_INIT(0x2A4D);
static const ble_uuid16_t protocol_mode_uuid = BLE_UUID16_INIT(0x2A4E);
static const ble_uuid16_t control_point_uuid = BLE_UUID16_INIT(0x2A4C);
static const ble_uuid16_t report_reference_uuid = BLE_UUID16_INIT(0x2908);
static const ble_uuid16_t battery_service_uuid = BLE_UUID16_INIT(0x180F);
static const ble_uuid16_t battery_level_uuid = BLE_UUID16_INIT(0x2A19);
static const ble_uuid16_t device_information_service_uuid =
    BLE_UUID16_INIT(0x180A);
static const ble_uuid16_t pnp_id_uuid = BLE_UUID16_INIT(0x2A50);

/* Game Pad, Report ID 1: signed 16-bit steering angle and pedal RPM. */
static const uint8_t hid_report_map[] = {
    0x05, 0x01,       /* Usage Page (Generic Desktop) */
    0x09, 0x05,       /* Usage (Game Pad) */
    0xA1, 0x01,       /* Collection (Application) */
    0x85, 0x01,       /* Report ID (1) */
    0x09, 0x30,       /* Usage (X) */
    0x16, 0xA6, 0xFF, /* Logical Minimum (-90) */
    0x26, 0x5A, 0x00, /* Logical Maximum (90) */
    0x75, 0x10,       /* Report Size (16) */
    0x95, 0x01,       /* Report Count (1) */
    0x81, 0x02,       /* Input (Data, Variable, Absolute) */
    0x09, 0x31,       /* Usage (Y) */
    0x16, 0x00, 0xFE, /* Logical Minimum (-512) */
    0x26, 0x00, 0x02, /* Logical Maximum (512) */
    0x75, 0x10,       /* Report Size (16) */
    0x95, 0x01,       /* Report Count (1) */
    0x81, 0x02,       /* Input (Data, Variable, Absolute) */
    0xC0,             /* End Collection */
};

static const uint8_t hid_information[] = {0x11, 0x01, 0x00, 0x02};
static const uint8_t report_reference[] = {0x01, 0x01};
static const uint8_t battery_level = CONFIG_BICIVER_BATTERY_LEVEL;
static const uint8_t pnp_id[] = {
    0x02,
    CONFIG_BICIVER_PNP_VENDOR_ID & 0xFF,
    (CONFIG_BICIVER_PNP_VENDOR_ID >> 8) & 0xFF,
    CONFIG_BICIVER_PNP_PRODUCT_ID & 0xFF,
    (CONFIG_BICIVER_PNP_PRODUCT_ID >> 8) & 0xFF,
    CONFIG_BICIVER_PNP_PRODUCT_VERSION & 0xFF,
    (CONFIG_BICIVER_PNP_PRODUCT_VERSION >> 8) & 0xFF,
};

static uint16_t hid_info_value_handle;
static uint16_t report_map_value_handle;
static uint16_t report_value_handle;
static uint16_t protocol_mode_value_handle;
static uint16_t control_point_value_handle;
static uint16_t battery_level_value_handle;
static uint16_t pnp_id_value_handle;
static uint16_t s_connection_handle = BLE_HS_CONN_HANDLE_NONE;
static bool s_report_subscribed;
static bool s_suspended;
static uint8_t s_protocol_mode = 1;
static bool s_force_notify;
static bool s_have_last_report;
static int16_t s_last_x;
static int16_t s_last_y;
static int64_t s_last_notify_us;
static portMUX_TYPE s_state_mux = portMUX_INITIALIZER_UNLOCKED;

static int append_bytes(struct ble_gatt_access_ctxt *ctxt, const void *data,
                        uint16_t length) {
  return os_mbuf_append(ctxt->om, data, length) == 0
             ? 0
             : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int read_report(uint8_t report[4], int16_t *x_out, int16_t *y_out) {
  generic_data_snapshot_t snapshot;
  if (generic_data_get_snapshot(&snapshot) != ESP_OK) {
    return BLE_ATT_ERR_UNLIKELY;
  }

  const generic_data_signal_t *angle_signal = NULL;
  const generic_data_signal_t *rpm_signal = NULL;
  for (uint8_t i = 0; i < snapshot.count; ++i) {
    if (snapshot.signals[i].id == GENERIC_SIGNAL_STEERING_ANGLE) {
      angle_signal = &snapshot.signals[i];
    } else if (snapshot.signals[i].id == GENERIC_SIGNAL_PEDAL_RPM) {
      rpm_signal = &snapshot.signals[i];
    }
  }
  if (angle_signal == NULL || rpm_signal == NULL ||
      angle_signal->type != GENERIC_DATA_TYPE_FLOAT ||
      rpm_signal->type != GENERIC_DATA_TYPE_INT32 || !angle_signal->valid ||
      !rpm_signal->valid) {
    return BLE_ATT_ERR_UNLIKELY;
  }

  float angle = angle_signal->value.f32;
  int32_t rpm = rpm_signal->value.i32;
  if (!isfinite(angle)) {
    return BLE_ATT_ERR_UNLIKELY;
  }
  if (angle < -90.0f) angle = -90.0f;
  if (angle > 90.0f) angle = 90.0f;
  if (rpm < -512) rpm = -512;
  if (rpm > 512) rpm = 512;

  int16_t x = (int16_t)lroundf(angle);
  int16_t y = (int16_t)rpm;
  report[0] = (uint8_t)(x & 0xFF);
  report[1] = (uint8_t)(((uint16_t)x >> 8) & 0xFF);
  report[2] = (uint8_t)(y & 0xFF);
  report[3] = (uint8_t)(((uint16_t)y >> 8) & 0xFF);
  if (x_out != NULL) *x_out = x;
  if (y_out != NULL) *y_out = y;
  return 0;
}

static int report_reference_access(uint16_t conn_handle, uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt *ctxt,
                                   void *arg) {
  (void)conn_handle;
  (void)attr_handle;
  (void)arg;
  if (ctxt->op != BLE_GATT_ACCESS_OP_READ_DSC) {
    return BLE_ATT_ERR_READ_NOT_PERMITTED;
  }
  return append_bytes(ctxt, report_reference, sizeof(report_reference));
}

static int hid_characteristic_access(uint16_t conn_handle,
                                     uint16_t attr_handle,
                                     struct ble_gatt_access_ctxt *ctxt,
                                     void *arg) {
  (void)conn_handle;
  (void)arg;
  if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    if (attr_handle == hid_info_value_handle) {
      return append_bytes(ctxt, hid_information, sizeof(hid_information));
    }
    if (attr_handle == report_map_value_handle) {
      return append_bytes(ctxt, hid_report_map, sizeof(hid_report_map));
    }
    if (attr_handle == report_value_handle) {
      uint8_t report[4];
      int rc = read_report(report, NULL, NULL);
      return rc == 0 ? append_bytes(ctxt, report, sizeof(report)) : rc;
    }
    if (attr_handle == protocol_mode_value_handle) {
      return append_bytes(ctxt, &s_protocol_mode, sizeof(s_protocol_mode));
    }
    if (attr_handle == battery_level_value_handle) {
      return append_bytes(ctxt, &battery_level, sizeof(battery_level));
    }
    if (attr_handle == pnp_id_value_handle) {
      return append_bytes(ctxt, pnp_id, sizeof(pnp_id));
    }
    return BLE_ATT_ERR_READ_NOT_PERMITTED;
  }

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    uint16_t length = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t value;
    if (length != 1 || os_mbuf_copydata(ctxt->om, 0, sizeof(value), &value) !=
                           0) {
      return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }
    if (attr_handle == protocol_mode_value_handle) {
      if (value != 1) {
        /* This gamepad implements Report Protocol and no Boot report. */
        return BLE_ATT_ERR_REQ_NOT_SUPPORTED;
      }
      s_protocol_mode = value;
      return 0;
    }
    if (attr_handle == control_point_value_handle) {
      if (value > 1) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
      }
      taskENTER_CRITICAL(&s_state_mux);
      s_suspended = (value == 0);
      if (!s_suspended) {
        s_force_notify = true;
      }
      taskEXIT_CRITICAL(&s_state_mux);
      return 0;
    }
  }
  return BLE_ATT_ERR_UNLIKELY;
}

static struct ble_gatt_dsc_def report_descriptors[] = {
    {.uuid = &report_reference_uuid.u,
     .att_flags = BLE_ATT_F_READ | BLE_ATT_F_READ_ENC,
     .access_cb = report_reference_access},
    {0},
};

static const struct ble_gatt_chr_def hid_characteristics[] = {
    {.uuid = &hid_info_uuid.u,
     .access_cb = hid_characteristic_access,
     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
     .val_handle = &hid_info_value_handle},
    {.uuid = &report_map_uuid.u,
     .access_cb = hid_characteristic_access,
     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
     .val_handle = &report_map_value_handle},
    {.uuid = &report_uuid.u,
     .access_cb = hid_characteristic_access,
     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC |
              BLE_GATT_CHR_F_NOTIFY,
     .descriptors = report_descriptors,
     .val_handle = &report_value_handle},
    {.uuid = &protocol_mode_uuid.u,
     .access_cb = hid_characteristic_access,
     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC |
              BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC |
              BLE_GATT_CHR_F_WRITE_NO_RSP,
     .val_handle = &protocol_mode_value_handle},
    {.uuid = &control_point_uuid.u,
     .access_cb = hid_characteristic_access,
     .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC |
              BLE_GATT_CHR_F_WRITE_NO_RSP,
     .val_handle = &control_point_value_handle},
    {0},
};

static const struct ble_gatt_chr_def battery_characteristics[] = {
    {.uuid = &battery_level_uuid.u,
     .access_cb = hid_characteristic_access,
     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
     .val_handle = &battery_level_value_handle},
    {0},
};

static const struct ble_gatt_chr_def device_information_characteristics[] = {
    {.uuid = &pnp_id_uuid.u,
     .access_cb = hid_characteristic_access,
     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
     .val_handle = &pnp_id_value_handle},
    {0},
};

static const struct ble_gatt_svc_def hid_services[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &hid_service_uuid.u,
     .characteristics = hid_characteristics},
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &battery_service_uuid.u,
     .characteristics = battery_characteristics},
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &device_information_service_uuid.u,
     .characteristics = device_information_characteristics},
    {0},
};

int gatt_svc_init(void) {
  ble_svc_gatt_init();
  ble_svc_gap_init();
  int rc = ble_svc_gap_device_name_set(BLE_DEVICE_NAME);
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to set GAP device name: %d", rc);
    return rc;
  }
  rc = ble_svc_gap_device_appearance_set(0x03C4); /* Gamepad */
  if (rc != 0) {
    ESP_LOGE(TAG, "Failed to set GAP gamepad appearance: %d", rc);
    return rc;
  }
  rc = ble_gatts_count_cfg(hid_services);
  if (rc != 0) {
    return rc;
  }
  return ble_gatts_add_svcs(hid_services);
}

void ble_hid_set_connection(uint16_t conn_handle) {
  taskENTER_CRITICAL(&s_state_mux);
  s_connection_handle = conn_handle;
  s_report_subscribed = false;
  s_suspended = false;
  s_force_notify = true;
  s_have_last_report = false;
  s_last_notify_us = 0;
  taskEXIT_CRITICAL(&s_state_mux);
}

void ble_gatt_svr_subscribe_hid_cb(struct ble_gap_event *event) {
  if (event->subscribe.attr_handle != report_value_handle) {
    return;
  }
  taskENTER_CRITICAL(&s_state_mux);
  s_report_subscribed = event->subscribe.cur_notify != 0;
  if (s_report_subscribed) {
    s_force_notify = true;
  }
  bool subscribed = s_report_subscribed;
  taskEXIT_CRITICAL(&s_state_mux);
  ESP_LOGI(TAG, "Input report notifications %s",
           subscribed ? "enabled" : "disabled");
}

esp_err_t ble_hid_notify_latest(void) {
  taskENTER_CRITICAL(&s_state_mux);
  uint16_t conn_handle = s_connection_handle;
  bool subscribed = s_report_subscribed;
  bool suspended = s_suspended;
  bool force_notify = s_force_notify;
  bool have_last_report = s_have_last_report;
  int16_t last_x = s_last_x;
  int16_t last_y = s_last_y;
  int64_t last_notify_us = s_last_notify_us;
  taskEXIT_CRITICAL(&s_state_mux);

  if (conn_handle == BLE_HS_CONN_HANDLE_NONE || !subscribed || suspended) {
    return ESP_ERR_INVALID_STATE;
  }
  struct ble_gap_conn_desc connection;
  if (ble_gap_conn_find(conn_handle, &connection) != 0 ||
      !connection.sec_state.encrypted || !connection.sec_state.bonded) {
    return ESP_ERR_INVALID_STATE;
  }

  uint8_t report[4];
  int16_t x;
  int16_t y;
  int rc = read_report(report, &x, &y);
  if (rc != 0) {
    return ESP_FAIL;
  }
  if (!force_notify && have_last_report && x == last_x && y == last_y) {
    return ESP_OK;
  }

  int64_t now_us = esp_timer_get_time();
  if (last_notify_us != 0 && now_us - last_notify_us < 20000) {
    return ESP_OK;
  }

  struct os_mbuf *om = ble_hs_mbuf_from_flat(report, sizeof(report));
  if (om == NULL) {
    return ESP_ERR_NO_MEM;
  }
  rc = ble_gatts_notify_custom(conn_handle, report_value_handle, om);
  if (rc != 0) {
    os_mbuf_free_chain(om);
    return ESP_FAIL;
  }
  taskENTER_CRITICAL(&s_state_mux);
  if (s_connection_handle == conn_handle) {
    s_last_notify_us = now_us;
    s_last_x = x;
    s_last_y = y;
    s_have_last_report = true;
    s_force_notify = false;
  }
  taskEXIT_CRITICAL(&s_state_mux);
  return ESP_OK;
}
