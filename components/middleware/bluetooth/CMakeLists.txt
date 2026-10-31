idf_component_register(SRCS "protocol/ble_gap.c" "ble_manager.c"
                              "protocol/ble_gatt.c"
                    INCLUDE_DIRS "include"
                    REQUIRES bt nvs_flash esp_timer generic-data)
