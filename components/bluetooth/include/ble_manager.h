#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *device_name;      /**< Full device name (GAP & Scan Response) */
  const char *short_name;       /**< Short name in Adv payload */
  uint16_t appearance;          /**< GAP appearance (e.g. 0x03C4 for Gamepad) */
  uint8_t battery_level;        /**< Battery level percentage (0-100) */
  uint16_t pnp_vendor_id;       /**< USB PnP Vendor ID */
  uint16_t pnp_product_id;      /**< USB PnP Product ID */
  uint16_t pnp_product_version; /**< USB PnP Product Version */
} ble_config_t;

#define BLE_CONFIG_DEFAULT() {             \
  .device_name = "BICIVER_TERAPEUTICA",     \
  .short_name = "BICIVER-HID",              \
  .appearance = 0x03C4,                     \
  .battery_level = 100,                     \
  .pnp_vendor_id = 0x303A,                  \
  .pnp_product_id = 0x0000,                 \
  .pnp_product_version = 0x0100,            \
}

/**
 * @brief Initialize BLE stack, NVS, HID service, and start advertising.
 * @param config Pointer to configuration struct, or NULL to use defaults.
 */
esp_err_t ble_manager_init(const ble_config_t *config);

/**
 * @brief Get active BLE configuration.
 */
const ble_config_t *ble_manager_get_config(void);

void ble_host_config_init(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_MANAGER_H
