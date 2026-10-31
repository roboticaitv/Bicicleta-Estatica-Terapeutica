idf_component_register(SRCS "ledc.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_common esp_driver_ledc log)
