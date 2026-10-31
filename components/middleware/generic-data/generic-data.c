#include "generic-data.h"

#include <math.h>
#include <string.h>

#include "esp_log.h"

static const char *TAG = "GENERIC_DATA";
static generic_data_snapshot_t s_data;

static int find_signal(uint16_t id) {
  for (uint8_t i = 0; i < s_data.count; ++i) {
    if (s_data.signals[i].id == id) {
      return i;
    }
  }
  return -1;
}

esp_err_t generic_data_init(void) {
  memset(&s_data, 0, sizeof(s_data));
  const generic_data_descriptor_t descriptors[] = {
      {.id = GENERIC_SIGNAL_STEERING_ANGLE,
       .name = "steering_angle",
       .unit = "deg",
       .type = GENERIC_DATA_TYPE_FLOAT},
      {.id = GENERIC_SIGNAL_PEDAL_RPM,
       .name = "pedal_rpm",
       .unit = "rpm",
       .type = GENERIC_DATA_TYPE_INT32},
  };
  const generic_data_value_t initial_values[] = {{.f32 = 0.0f}, {.i32 = 0}};

  for (size_t i = 0; i < sizeof(descriptors) / sizeof(descriptors[0]); ++i) {
    esp_err_t err = generic_data_register(&descriptors[i], initial_values[i]);
    if (err != ESP_OK) {
      return err;
    }
  }
  ESP_LOGI(TAG, "Registered %u sensor signals", s_data.count);
  return ESP_OK;
}

esp_err_t generic_data_register(const generic_data_descriptor_t *descriptor,
                                generic_data_value_t initial_value) {
  if (descriptor == NULL || descriptor->id == 0 || descriptor->name == NULL ||
      descriptor->unit == NULL || find_signal(descriptor->id) >= 0 ||
      s_data.count >= GENERIC_DATA_MAX_SIGNALS ||
      strlen(descriptor->name) >= GENERIC_DATA_NAME_MAX_LEN ||
      strlen(descriptor->unit) >= GENERIC_DATA_UNIT_MAX_LEN) {
    return ESP_ERR_INVALID_ARG;
  }
  if (descriptor->type < GENERIC_DATA_TYPE_FLOAT ||
      descriptor->type > GENERIC_DATA_TYPE_BOOL) {
    return ESP_ERR_INVALID_ARG;
  }
  if (descriptor->type == GENERIC_DATA_TYPE_FLOAT &&
      !isfinite(initial_value.f32)) {
    return ESP_ERR_INVALID_ARG;
  }

  generic_data_signal_t *signal = &s_data.signals[s_data.count++];
  signal->id = descriptor->id;
  strlcpy(signal->name, descriptor->name, sizeof(signal->name));
  strlcpy(signal->unit, descriptor->unit, sizeof(signal->unit));
  signal->type = descriptor->type;
  signal->value = initial_value;
  signal->valid = true;
  signal->revision = ++s_data.revision;
  return ESP_OK;
}

static esp_err_t set_value(uint16_t id, generic_data_type_t type,
                           generic_data_value_t value) {
  int index = find_signal(id);
  if (index < 0) {
    return ESP_ERR_NOT_FOUND;
  }
  generic_data_signal_t *signal = &s_data.signals[index];
  if (signal->type != type) {
    return ESP_ERR_INVALID_ARG;
  }
  if (type == GENERIC_DATA_TYPE_FLOAT && !isfinite(value.f32)) {
    return ESP_ERR_INVALID_ARG;
  }
  if (memcmp(&signal->value, &value, sizeof(value)) != 0 || !signal->valid) {
    signal->value = value;
    signal->valid = true;
    signal->revision = ++s_data.revision;
  }
  return ESP_OK;
}

esp_err_t generic_data_set_float(uint16_t id, float value) {
  return set_value(id, GENERIC_DATA_TYPE_FLOAT,
                   (generic_data_value_t){.f32 = value});
}

esp_err_t generic_data_set_int32(uint16_t id, int32_t value) {
  return set_value(id, GENERIC_DATA_TYPE_INT32,
                   (generic_data_value_t){.i32 = value});
}

esp_err_t generic_data_set_bool(uint16_t id, bool value) {
  return set_value(id, GENERIC_DATA_TYPE_BOOL,
                   (generic_data_value_t){.boolean = value});
}

esp_err_t generic_data_get(uint16_t id, generic_data_signal_t *signal) {
  if (signal == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  int index = find_signal(id);
  if (index < 0) {
    return ESP_ERR_NOT_FOUND;
  }
  *signal = s_data.signals[index];
  return ESP_OK;
}

esp_err_t generic_data_get_snapshot(generic_data_snapshot_t *snapshot) {
  if (snapshot == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  *snapshot = s_data;
  return ESP_OK;
}
