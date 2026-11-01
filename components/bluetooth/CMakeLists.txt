idf_component_register(SRCS "src/ble_gap.c" "src/ble_manager.c" "src/ble_gatt.c"
                       INCLUDE_DIRS "include"
                       REQUIRES bt esp_common esp_timer freertos log nvs_flash)
