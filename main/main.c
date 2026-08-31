/**
 * @file main.c
 * @brief Application entry point for Bicicleta Estática Terapéutica.
 */

#include "steering_wheel.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "Starting Bicicleta Estática Terapéutica application...");

    // Initialize steering wheel sensor module
    if (steering_wheel_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize steering wheel!");
    }

    // Main application loop
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));
        float angle = steering_wheel_get_angle_degrees();
        ESP_LOGD(TAG, "Current angle: %.2f deg", angle);
    }
}
