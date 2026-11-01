idf_component_register(SRCS "src/steering_wheel.c"
                       INCLUDE_DIRS "include"
                       REQUIRES adc esp_adc esp_common esp_driver_gpio log)
