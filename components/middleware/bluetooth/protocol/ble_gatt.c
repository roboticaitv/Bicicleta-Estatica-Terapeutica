#include "host/ble_gatt.h"
#include "services/gatt/ble_svc_gatt.h"
int gatt_svc_init(){
    int rc=0;

    ble_svc_gatt_init();

    struct ble_gatt_svc gatt_svr_svcs;
    //rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc!=0) {
        return rc;
    }

    //rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc!=0) {
        return rc;
    }


    return 0;
}

