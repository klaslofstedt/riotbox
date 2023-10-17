/*
 * ble.c
 *
 *  Created on: 25 may 2023
 *      Author: klaslofstedt
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "middlewares/ble.h"
#include "utilities/event.h"
#include "utilities/misc.h"
#include "utilities/aes.h"
#include "drivers/storage.h"

#define BLE_MAX_SERVICES 2

#define BLE_STORAGE_KEY_FLAGS       "ble_flags"
#define BLE_STORAGE_KEY_POP         "ble_pop"

static const char *TAG = "BLE";

static uint8_t ble_addr_type;
static uint16_t ble_connection_handle;
char ble_receive_buffer[BLE_BUFFER_SIZE];
static bool ble_allow_connection = false;

static int ble_event_handler(struct ble_gap_event *event, void *arg);
static bool ble_set_has_pop(void);
static void ble_app_on_sync(void);
static void ble_host_task(void *param);
static void ble_start(void);


// Array to store all the service definitions
static struct ble_gatt_svc_def services[BLE_MAX_SERVICES + 1] = {0};  // One extra for the ending {0}

static int ble_event_handler(struct ble_gap_event *event, void *arg)
{
    ble_connection_handle = event->connect.conn_handle;

    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_start();
        } else {
            event_trigger(EVENT_BLE_GAP_CONNECTED);
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "BLE GAP EVENT ADV COMPLETE");
        ble_start();
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "BLE GAP EVENT DISCONNECT");
        ble_start();
        event_trigger(EVENT_BLE_GAP_DISCONNECTED);
        break;
    case BLE_GAP_EVENT_NOTIFY_TX:
        ESP_LOGI(TAG, "BLE_GAP_EVENT_NOTIFY_TX");
        event_trigger(EVENT_BLE_NOTIFY_DONE);

        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
static void ble_start(void)
{
    ESP_LOGI(TAG, "Start advertising!");
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name();
    ESP_LOGI(TAG, "Device name: %s", device_name);

    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_event_handler, NULL);
}

static void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_start();
}

static void ble_host_task(void *param)
{
    nimble_port_run();
}

static bool ble_set_has_pop(void)
{
    return storage_set_flags(BLE_STORAGE_KEY_FLAGS, BLE_HAS_POP);
}

void ble_set_allow_connection(bool is_allowed)
{
    ble_allow_connection = is_allowed;
}

bool ble_is_connection_allowed(void)
{
    return ble_allow_connection;
}

bool ble_get_has_pop(void)
{
    return storage_has_flags(BLE_STORAGE_KEY_FLAGS, BLE_HAS_POP);
}

bool ble_set_pop(const char* pop)
{
    if (ble_get_has_pop()){
        ESP_LOGE(TAG, "Error: ble_get_has_pop");
        return false;
    }
    if (!storage_set_blob(BLE_STORAGE_KEY_POP, pop, strlen(pop) + 1)){
        ESP_LOGE(TAG, "Error: storage_set_blob");
        return false;
    }
    return ble_set_has_pop();
}

bool ble_get_pop(char* pop)
{
    if (!ble_get_has_pop()){
        ESP_LOGE(TAG, "Error: ble_get_has_pop");
        return false;
    }
    return storage_get_blob(BLE_STORAGE_KEY_POP, pop, BLE_POP_SIZE * 2 + 1);
}

bool ble_register_service(struct ble_gatt_svc_def service) 
{
    // Find the next free slot in the array and add the service.
    for (int i = 0; i < BLE_MAX_SERVICES; i++) {
        if (services[i].type == 0) {  // Check the type of the service for a free slot
            services[i] = service;

            // Ensure the next service is zeroed out to indicate the end of the array.
            if (i + 1 < BLE_MAX_SERVICES + 1) {
                services[i + 1] = (struct ble_gatt_svc_def) {0};
            }
            return true;
        }
    }
    // Return false if there were no available slot in "services".
    ESP_LOGE(TAG, "Error: No service slots available");
    return false;
}

bool ble_send_notification(uint16_t ble_notification_handle, char* json_str) 
{
    struct os_mbuf *om;
    om = ble_hs_mbuf_from_flat(json_str, strlen(json_str));
    // TODO: come up with a better way to handle ble_connection_handle
    ble_connection_handle = 0;
    ESP_LOGI(TAG, "Notifying conn=%d", ble_connection_handle);
    int rc = ble_gatts_notify_custom(ble_connection_handle, ble_notification_handle, om);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error: Notify failed %d", rc);
        return false;
    }
    return true;
}

bool ble_init(void) 
{
    ESP_LOGI(TAG, "Initialise");
    char thing_id[ID_SIZE];
    if (!id_get(thing_id)){
        ESP_LOGE(TAG, "Error: id_get");
        return false;
    }
    nimble_port_init();
    if (ble_svc_gap_device_name_set(thing_id) != 0){
        ESP_LOGE(TAG, "Error: ble_svc_gap_device_name_set");
        return false;
    }
    ble_svc_gap_init();
    ble_svc_gatt_init();

    if (ble_gatts_count_cfg(services) != 0){
        ESP_LOGE(TAG, "Error: ble_gatts_count_cfg");
        return false;
    }
    if (ble_gatts_add_svcs(services) != 0){
        ESP_LOGE(TAG, "Error: ble_gatts_add_svcs");
        return false;
    }

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(ble_host_task);

    return true;
}
