#ifndef LEDC_HAL_H
#define LEDC_HAL_H

#include <stdint.h>
#include "driver/ledc.h"

void ledc_init(ledc_mode_t, ledc_timer_t, ledc_channel_t, int);
void ledc_new_tick_asreason(ledc_mode_t, ledc_channel_t, int16_t, int16_t);
void ledc_new_tick_asper(ledc_mode_t, ledc_channel_t, float);

#endif