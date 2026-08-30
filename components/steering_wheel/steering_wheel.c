#include "steering_wheel.h"
#include "hal/adc_types.h"
#include "hw_adc.h"

void initSteeringWheel(void) {
  //unit1,gpio4
  hal_adc_t *handler = init(ADC_UNIT_1, ADC_CHANNEL_3);
  

}

void getAngleDegrees(){

}

void calibrateCenter(){
  
}