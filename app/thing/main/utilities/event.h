/*
 * event.h
 *
 *  Created on: 20 jun 2023
 *      Author: klaslofstedt
 */

#ifndef _EVENT_H_
#define _EVENT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef enum
{
    // Default event
    EVENT_IGNORE = BIT0,
    // BLE events
    EVENT_BLE_GAP_CONNECTED = BIT1,
    EVENT_BLE_GAP_DISCONNECTED = BIT2,
    EVENT_BLE_NOTIFY_DONE = BIT3,
    // Wifi events
    EVENT_WIFI_START = BIT4,
    EVENT_WIFI_DISCONNECTED = BIT5,
    EVENT_WIFI_CONNECTED = BIT6,
    EVENT_WIFI_SCAN_DONE = BIT7,
    // MQTT events
    EVENT_MQTT_CONNECTED = BIT8,
    EVENT_MQTT_SUBSCRIBED = BIT9,
    EVENT_MQTT_DATA_RECEIVED = BIT10,
    // new Provision
    EVENT_PROVISION_NOTIFYING_WIFI_SCAN = BIT11,
    EVENT_PROVISION_NOTIFYING_STATUS = BIT12,
    EVENT_PROVISION_RECEIVE_POP = BIT13,
    EVENT_PROVISION_RECEIVE_ROOT_CA = BIT14,
    EVENT_PROVISION_RECEIVE_THING_CERT = BIT15,
    EVENT_PROVISION_RECEIVE_THING_KEY = BIT16,
    EVENT_PROVISION_RECEIVE_WIFI_CREDS = BIT17,
    // Thing events
    EVENT_THING_RECEIVED_OTAURL = BIT18,
    EVENT_THING_RECEIVED_VALUE = BIT19,
    EVENT_THING_RECEIVED_BOOTUP = BIT20,
    EVENT_THING_PUBLISH_OTAURL = BIT21,
    EVENT_THING_PUBLISH_VALUE = BIT22,
    EVENT_THING_PUBLISH_BOOTUP = BIT23,
} events_t;

bool event_init(void);
events_t event_wait(void);
void event_expect(events_t events);
bool event_trigger(events_t events);

#endif /* _EVENT_H_ */

