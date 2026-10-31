/**
 * @file main.c
 * @brief Application entry point for Bicicleta Estática Terapéutica.
 */

#include "esp_err.h"
#include "hal/ledc_types.h"
#include "ledc.h"
#include "steering_wheel.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ble_manager.h"

static const char *TAG = "APP_MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "Biciver Init...");

    // Checar cual gpin es ideal
    ledc_init(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, LEDC_CHANNEL_1, 10);

    // Initialize steering wheel sensor module
    if (steering_wheel_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize steering wheel!");
    }

    // Initilize bluetooth service
    if (ble_manager_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Nimble component");
    }
    // Main application loop
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));
        float angle = steering_wheel_get_angle_degrees();
        ESP_LOGD(TAG, "Current angle: %.2f deg", angle);
    }

}
