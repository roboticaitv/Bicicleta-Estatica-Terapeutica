idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES steering_wheel bluetooth esp_common freertos generic-data log)
