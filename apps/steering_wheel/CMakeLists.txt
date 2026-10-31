idf_component_register(SRCS "steering_wheel.c"
                       INCLUDE_DIRS "."
                       REQUIRES adc esp_adc esp_common esp_driver_gpio log)

