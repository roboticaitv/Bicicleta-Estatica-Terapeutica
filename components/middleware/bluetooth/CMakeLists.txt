idf_component_register(SRCS "protocol/ble_gap.c" "ble_manager.c"
                              "protocol/ble_gatt.c"
                    INCLUDE_DIRS "include"
                    REQUIRES bt esp_common esp_timer freertos generic-data log nvs_flash)
