idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES steering_wheel data_manager comm_manager bluetooth esp_common freertos log)
