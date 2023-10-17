/*
 * event.c
 *
 *  Created on: 20 jun 2023
 *      Author: klaslofstedt
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "utilities/event.h"

// Has to be a large number as e.g. the BLE task has higher prio and will send lots of notifications per message
#define EVENTS_MAX_IN_QUEUE     50
static const char *TAG = "EVENT";

static const char *event_string(events_t events);

static QueueHandle_t events_handle;
static events_t events_do_next = EVENT_IGNORE;


events_t event_wait(void)
{
    ESP_LOGI(TAG, "Waiting for events...");
    events_t event_triggered;
    xQueueReceive(events_handle, &event_triggered, portMAX_DELAY);
    events_t event_do = event_triggered & events_do_next;
    events_t event_ignore = event_triggered & ~events_do_next;

    if (event_do) {
        ESP_LOGI(TAG, "DO -> %s", event_string(event_do));
        return event_do;
    } else {
        ESP_LOGI(TAG, "IGNORE -> %s", event_string(event_ignore));
        return EVENT_IGNORE;
    }
}

void event_expect(events_t events)
{
    events_do_next = events;
}

bool event_trigger(events_t event)
{
    if (xQueueSendToBack(events_handle, &event, 0) != pdTRUE){
        ESP_LOGE(TAG, "Error: Failed to trigger -> %s", event_string(event));
        return false;
    }
    ESP_LOGI(TAG, "TRIGGER -> %s", event_string(event));
    return true;
}

static const char* event_string(events_t events)
{
    switch(events) {
        // Default event
        case EVENT_IGNORE: return "EVENT_IGNORE ";
        // BLE events
        case EVENT_BLE_GAP_CONNECTED: return "EVENT_BLE_GAP_CONNECTED ";
        case EVENT_BLE_GAP_DISCONNECTED: return "EVENT_BLE_GAP_DISCONNECTED ";
        case EVENT_BLE_NOTIFY_DONE: return "EVENT_BLE_NOTIFY_DONE ";
        // Wifi events
        case EVENT_WIFI_START: return "EVENT_WIFI_START ";
        case EVENT_WIFI_DISCONNECTED: return "EVENT_WIFI_DISCONNECTED ";
        case EVENT_WIFI_CONNECTED: return "EVENT_WIFI_CONNECTED ";
        case EVENT_WIFI_SCAN_DONE: return "EVENT_WIFI_SCAN_DONE ";
        // MQTT events
        case EVENT_MQTT_CONNECTED: return "EVENT_MQTT_CONNECTED ";
        case EVENT_MQTT_SUBSCRIBED: return "EVENT_MQTT_SUBSCRIBED ";
        case EVENT_MQTT_DATA_RECEIVED: return "EVENT_MQTT_DATA_RECEIVED ";
        // Provisioning events
        case EVENT_PROVISION_NOTIFYING_WIFI_SCAN: return "EVENT_PROVISION_NOTIFYING_WIFI_SCAN ";
        case EVENT_PROVISION_NOTIFYING_STATUS: return "EVENT_PROVISION_NOTIFYING_STATUS ";
        case EVENT_PROVISION_RECEIVE_POP: return "EVENT_PROVISION_RECEIVE_POP ";
        case EVENT_PROVISION_RECEIVE_WIFI_CREDS: return "EVENT_PROVISION_RECEIVE_WIFI_CREDS ";
        case EVENT_PROVISION_RECEIVE_ROOT_CA: return "EVENT_PROVISION_RECEIVE_ROOT_CA ";
        case EVENT_PROVISION_RECEIVE_THING_CERT: return "EVENT_PROVISION_RECEIVE_THING_CERT ";
        case EVENT_PROVISION_RECEIVE_THING_KEY: return "EVENT_PROVISION_RECEIVE_THING_KEY ";
        // Thing events
        case EVENT_THING_RECEIVED_OTAURL: return "EVENT_THING_RECEIVED_OTAURL ";
        case EVENT_THING_RECEIVED_VALUE: return "EVENT_THING_RECEIVED_VALUE "; 
        case EVENT_THING_RECEIVED_BOOTUP: return "EVENT_THING_RECEIVED_BOOTUP "; 
        case EVENT_THING_PUBLISH_OTAURL: return "EVENT_THING_PUBLISH_OTAURL ";
        case EVENT_THING_PUBLISH_VALUE: return "EVENT_THING_PUBLISH_VALUE ";
        case EVENT_THING_PUBLISH_BOOTUP: return "EVENT_THING_PUBLISH_BOOTUP ";

        default: return "UNKNOWN_APP_EVENT ";
    }
}

bool event_init(void)
{
    ESP_LOGI(TAG, "Initialise");
    events_handle = xQueueCreate(EVENTS_MAX_IN_QUEUE, sizeof(events_t));
    if (events_handle == NULL){
        ESP_LOGE(TAG, "Error: xQueueCreate");
        return false;
    }
    return true;
}