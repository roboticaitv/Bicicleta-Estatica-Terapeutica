idf_component_register(SRCS "src/comm_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES bluetooth data_manager esp_common freertos log)
