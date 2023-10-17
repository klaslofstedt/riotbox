#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "app/ota.h"
#include "drivers/storage.h"
#include "middlewares/wifi.h"
#include "middlewares/ble.h"
#include "utilities/misc.h"
#include "utilities/aes.h"
#include <string.h>
#include <stdio.h>
#include "middlewares/mqtt.h"
#include "app/thing.h"
#include "utilities/event.h"
#include "utilities/state.h"
#include "app/deploy.h"
#include "middlewares/auth.h"
#include "app/provision.h"
#include "utilities/auth_aws_provision.h"
#include "esp_log.h"

static const char *TAG = "APP";

void app_main(void)
{
    bool ok = true;
    if (ok){
        ok = event_init();
    }
    if (ok){
        ok = state_init();
    }
    if (ok){
        ok = storage_init();
    }
    if (ok){
        ok = deploy_init();
    }
    if (ok){
        ok = aes_init();
    }
    if (ok){
        ok = auth_init();
    }
    if (ok){
        ok = wifi_init();
    }
    if (ok){
        ok = provision_init();
    }
    if (ok){
        ok = ble_init();
    }
    if (ok){
        ok = thing_init();
    }
    if (ok){
        ok = mqtt_init();
    }

    // Wait for either BLE or WiFi events on boot
    event_expect(EVENT_BLE_GAP_CONNECTED | EVENT_WIFI_START);
    // Initialise state machine to Thing operation on boot
    state_set(STATE_THING);

    ESP_LOGI(TAG, "App version: %s: ", PROJECT_VER);
    
    while (ok){
        switch (state_get()){
            case STATE_PROVISION:
                ok = provision_run();
                break;
            case STATE_OTA:
                ok = ota_run();
                break;
            case STATE_THING:
                ok = thing_run();
                break;
            default:
                ok = false;
                break;
        }
    }

    thing_reboot();
}
