/**
 * @file steering_wheel.c
 * @brief Implementation of the steering wheel feature logic.
 */

#include "steering_wheel.h"
#include "adc.h"
#include "esp_log.h"

static const char *TAG = "STEERING_WHEEL";

static hal_adc_t *s_adc_handle;

static steering_wheel_config_t s_config = {
    .adc_min_raw = 0,
    .adc_max_raw = 4095,
    .adc_center_raw = 2048,
    .max_angle_deg = 45.0f
};

esp_err_t steering_wheel_init(void) {
    // GPIO 4 corresponds to ADC_UNIT_1, ADC_CHANNEL_3 on ESP32
    s_adc_handle = hw_adc_init(ADC_UNIT_1, ADC_CHANNEL_3);
    if (!s_adc_handle) {
        ESP_LOGE(TAG, "Failed to initialize ADC for steering wheel");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Steering wheel module initialized successfully on ADC1_CH3");
    return ESP_OK;
}

float steering_wheel_get_angle_degrees(void) {
    if (!s_adc_handle) {
        ESP_LOGW(TAG, "Steering wheel not initialized. Calling steering_wheel_init().");
        if (steering_wheel_init() != ESP_OK) {
            return 0.0f;
        }
    }

    int raw_val = 0;
    readRaw12Bit(s_adc_handle, &raw_val);

    // Compute angle relative to center:
    // Left: raw < center, Right: raw > center
    float angle = 0.0f;
    if (raw_val >= s_config.adc_center_raw) {
        int span = s_config.adc_max_raw - s_config.adc_center_raw;
        if (span > 0) {
            angle = ((float)(raw_val - s_config.adc_center_raw) / (float)span) * s_config.max_angle_deg;
        }
    } else {
        int span = s_config.adc_center_raw - s_config.adc_min_raw;
        if (span > 0) {
            angle = -((float)(s_config.adc_center_raw - raw_val) / (float)span) * s_config.max_angle_deg;
        }
    }

    ESP_LOGI(TAG, "Steering Angle: %.2f deg (Raw: %d)", angle, raw_val);
    return angle;
}

esp_err_t steering_wheel_calibrate_center(void) {
    if (!s_adc_handle) {
        return ESP_FAIL;
    }

    int raw_val = 0;
    readRaw12Bit(s_adc_handle, &raw_val);
    s_config.adc_center_raw = raw_val;
    ESP_LOGI(TAG, "Steering center calibrated to raw value: %d", raw_val);
    return ESP_OK;
}

/* ========================================================================= */
/* Legacy Wrappers                                                           */
/* ========================================================================= */

void initSteeringWheel(void) {
    steering_wheel_init();
}

void getAngleDegrees(void) {
    (void)steering_wheel_get_angle_degrees();
}

void calibrateCenter(void) {
    steering_wheel_calibrate_center();
}
