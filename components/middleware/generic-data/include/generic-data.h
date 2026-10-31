#ifndef GENERIC_DATA_H
#define GENERIC_DATA_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GENERIC_DATA_MAX_SIGNALS 16
#define GENERIC_DATA_NAME_MAX_LEN 24
#define GENERIC_DATA_UNIT_MAX_LEN 12

/* IDs are stable across firmware versions so consumers do not depend on slots. */
typedef enum {
  GENERIC_SIGNAL_STEERING_ANGLE = 1,
  GENERIC_SIGNAL_PEDAL_RPM = 2,
} generic_data_id_t;

typedef enum {
  GENERIC_DATA_TYPE_FLOAT = 1,
  GENERIC_DATA_TYPE_INT32,
  GENERIC_DATA_TYPE_BOOL,
} generic_data_type_t;

typedef union {
  float f32;
  int32_t i32;
  bool boolean;
} generic_data_value_t;

typedef struct {
  uint16_t id;
  const char *name;
  const char *unit;
  generic_data_type_t type;
} generic_data_descriptor_t;

typedef struct {
  uint16_t id;
  char name[GENERIC_DATA_NAME_MAX_LEN];
  char unit[GENERIC_DATA_UNIT_MAX_LEN];
  generic_data_type_t type;
  generic_data_value_t value;
  bool valid;
  uint32_t revision;
} generic_data_signal_t;

typedef struct {
  generic_data_signal_t signals[GENERIC_DATA_MAX_SIGNALS];
  uint8_t count;
  uint32_t revision;
} generic_data_snapshot_t;

esp_err_t generic_data_init(void);
esp_err_t generic_data_register(const generic_data_descriptor_t *descriptor,
                                generic_data_value_t initial_value);
esp_err_t generic_data_set_float(uint16_t id, float value);
esp_err_t generic_data_set_int32(uint16_t id, int32_t value);
esp_err_t generic_data_set_bool(uint16_t id, bool value);
esp_err_t generic_data_get(uint16_t id, generic_data_signal_t *signal);
esp_err_t generic_data_get_snapshot(generic_data_snapshot_t *snapshot);

#ifdef __cplusplus
}
#endif

#endif
