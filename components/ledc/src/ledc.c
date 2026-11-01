#include "ledc.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/ledc_types.h"

static const char* TAG = "ledc-hal";

void ledc_init(ledc_mode_t LEDC_MODE, ledc_timer_t LEDC_TIMER, ledc_channel_t LEDC_CHANNEL, int GPIO_NUM) {
    ledc_timer_config_t ledc_timer_cfg = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .freq_hz = 20000,
        .clk_cfg = LEDC_AUTO_CLK,
        .duty_resolution = LEDC_TIMER_9_BIT,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_cfg));

    ledc_channel_config_t ledc_channel_cfg = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .gpio_num = GPIO_NUM,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_cfg));
}

static void ledc_new_tick(ledc_mode_t LEDC_MODE, ledc_channel_t LEDC_CHANNEL, uint32_t DUTY) {
    ESP_LOGI(TAG, "duty: %lu channel: %d", (unsigned long)DUTY, LEDC_CHANNEL);

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

void ledc_new_tick_asreason(ledc_mode_t LEDC_MODE, ledc_channel_t LEDC_CHANNEL, int16_t val, int16_t maxval) {
    if (maxval <= 0) {
        return;
    }

    if (val < 0) {
        val = 0;
    }

    if (val > maxval) {
        val = maxval;
    }

    uint32_t duty = ((uint32_t)val * 511U) / (uint32_t)maxval;

    ledc_new_tick(LEDC_MODE, LEDC_CHANNEL, duty);
}

void ledc_new_tick_asper(ledc_mode_t LEDC_MODE, ledc_channel_t LEDC_CHANNEL, float per) {
    if (per < 0.0f) {
        per = 0.0f;
    }

    if (per > 100.0f) {
        per = 100.0f;
    }

    uint32_t duty = (uint32_t)(per * 511.0f / 100.0f);

    ledc_new_tick(LEDC_MODE, LEDC_CHANNEL, duty);
}