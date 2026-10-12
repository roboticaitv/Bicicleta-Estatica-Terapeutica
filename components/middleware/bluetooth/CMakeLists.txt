idf_component_register(SRCS "protocol/ble_gap.c" "ble_manager.c"
                    INCLUDE_DIRS "include"
                    REQUIRES bt nvs_flash)
