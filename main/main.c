#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "hw_adc.h"
#include <stdio.h>

void app_main(void) {
  hal_adc_t *handler = init(ADC_UNIT_1, ADC_CHANNEL_3);
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(150));
    readRaw12Bit(handler);
  }
}
