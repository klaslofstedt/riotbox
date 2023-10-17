/*
 * ble.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _BLE_H_
#define _BLE_H_

/*nimBLE Host*/
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define BLE_POP_SIZE 16
#define BLE_BUFFER_SIZE 256

typedef enum
{
    BLE_HAS_POP = BIT0,
} ble_flags_t;

bool ble_register_service(struct ble_gatt_svc_def service);
bool ble_init(void);
bool ble_set_pop(const char *pop);
bool ble_get_pop(char *pop);
bool ble_get_has_pop(void);
bool ble_send_notification(uint16_t ble_notification_handle, char *json_str);
void ble_set_allow_connection(bool is_allowed);
bool ble_is_connection_allowed(void);

#endif /* _BLE_H_ */
