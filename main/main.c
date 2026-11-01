#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ble_manager.h"
#include "steering_wheel.h"
#include "data_manager.h"
#include "comm_manager.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
  ESP_LOGI(TAG, "Starting Bicicleta-Estatica-Terapeutica...");

  // 1. Initialize drivers & hardware modules
  ESP_ERROR_CHECK(steering_wheel_init());
  static const ble_config_t ble_cfg = BLE_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(ble_manager_init(&ble_cfg));

  // 2. Initialize application managers
  ESP_ERROR_CHECK(data_manager_init());
  ESP_ERROR_CHECK(comm_manager_init());

  ESP_LOGI(TAG, "All subsystems running. Entering superloop...");

  while (true) {
    // INPUT: Sample sensors, filter noise & deadbands, update mailbox
    data_manager_tick();

    // OUTPUT: Format host packets, dispatch over BLE
    comm_manager_tick();

    // 50 Hz tick (20 ms)
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
