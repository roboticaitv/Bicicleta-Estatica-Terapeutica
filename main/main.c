#include "steering_wheel.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
  initSteeringWheel();
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(100));
    getAngleDegrees();
  }
}
