/*
 * provision.c
 *
 *  Created on: 24 jun 2023
 *      Author: klaslofstedt
 */
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include "app/provision.h"
#include "drivers/storage.h"
#include "middlewares/wifi.h"
#include "middlewares/mqtt.h"
#include "utilities/auth_aws_provision.h"
#include "utilities/event.h"
#include "middlewares/ble.h"
#include "utilities/state.h"
#include <string.h>
#include "utilities/aes.h"
#include <cJSON.h>
#include "esp_log.h"
#include "middlewares/auth.h"


/*nimBLE Host*/
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_uuid.h"
#include "host/ble_gatt.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"


#define PROVISION_SERVICE_UUID      0xFFFF
// Write from mobile to esp32
#define CHAR_WRITE_POP_UUID         0xFF01
#define CHAR_WRITE_WIFI_CREDS_UUID  0xFF02
#define CHAR_WRITE_ROOT_CA_UUID     0xFF03
#define CHAR_WRITE_THING_CERT_UUID  0xFF04
#define CHAR_WRITE_THING_KEY_UUID   0xFF05
// Notify mobile from esp32
#define CHAR_NOTIFY_UUID            0xFF06

static const char *TAG = "PROVISION";

static uint16_t handle_notification;
static char provision_ble_buffer[BLE_BUFFER_SIZE];
static size_t provision_ble_buffer_size = 0;

typedef enum
{
    BLOB_PROGRESS =   BIT0,
    BLOB_FAIL =       BIT1,
    BLOB_DONE =       BIT2,
} blob_status_t;

typedef enum
{
    PROV_PROGRESS =   BIT0,
    PROV_FAIL =       BIT1,
    PROV_DONE =       BIT2,
} prov_status_t;

static int provision_cb_pop(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int provision_cb_wifi_credentials(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int provision_cb_root_ca(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int provision_cb_thing_certificate(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int provision_cb_thing_key(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int provision_dummy_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static bool provision_register_ble_service(void);
static bool provision_notify_status(prov_status_t status);
static bool provision_notify_wifi_scan(void);
static bool provision_create_network_message(wifi_ap_record_t ap_record, uint16_t count, char* buffer, size_t size);
static bool provision_create_status_message(prov_status_t status, char* buffer, size_t size);
static bool provision_set_pop(void);
static bool provision_set_wifi_creds(void);
static blob_status_t provision_set_root_ca(void);
static blob_status_t provision_set_thing_cert(void);
static blob_status_t provision_set_thing_key(void);


const struct ble_gatt_svc_def provision_service[] = {
    {   .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(PROVISION_SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_WRITE_POP_UUID),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = provision_cb_pop
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_WRITE_WIFI_CREDS_UUID),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = provision_cb_wifi_credentials
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_WRITE_ROOT_CA_UUID),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = provision_cb_root_ca
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_WRITE_THING_CERT_UUID),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = provision_cb_thing_certificate
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_WRITE_THING_KEY_UUID),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = provision_cb_thing_key
            },
            {
                .uuid = BLE_UUID16_DECLARE(CHAR_NOTIFY_UUID),
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &handle_notification,
                .access_cb = provision_dummy_cb
            },
            {0}
        }
    },
    {0}
};

static int provision_dummy_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    return 0;
}

static int provision_cb_pop(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)handle_connection;
    (void)attr_handle;
    (void)arg;

    memset(provision_ble_buffer, 0, sizeof(provision_ble_buffer));
    memcpy(provision_ble_buffer,ctxt->om->om_data,ctxt->om->om_len);
    provision_ble_buffer_size = ctxt->om->om_len;

    event_trigger(EVENT_PROVISION_RECEIVE_POP);

    return 0;
}

static int provision_cb_wifi_credentials(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)handle_connection;
    (void)attr_handle;
    (void)arg;

    if (!ble_is_connection_allowed()){
        ESP_LOGE(TAG, "Error: BLE connection not allowed (provision_cb_wifi_credentials)");
        return 0;
    }

    memset(provision_ble_buffer, 0, sizeof(provision_ble_buffer));
    memcpy(provision_ble_buffer,ctxt->om->om_data,ctxt->om->om_len);
    provision_ble_buffer_size = ctxt->om->om_len;

    event_trigger(EVENT_PROVISION_RECEIVE_WIFI_CREDS);

    return 0;
}

static int provision_cb_root_ca(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)handle_connection;
    (void)attr_handle;
    (void)arg;

    if (!ble_is_connection_allowed()){
        ESP_LOGE(TAG, "Error: BLE connection not allowed (provision_cb_root_ca)");
        return 0;
    }
    memset(provision_ble_buffer, 0, sizeof(provision_ble_buffer));
    memcpy(provision_ble_buffer,ctxt->om->om_data,ctxt->om->om_len);
    provision_ble_buffer_size = ctxt->om->om_len;

    event_trigger(EVENT_PROVISION_RECEIVE_ROOT_CA);

    return 0;
}

static int provision_cb_thing_certificate(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)handle_connection;
    (void)attr_handle;
    (void)arg;

    if (!ble_is_connection_allowed()){
        ESP_LOGE(TAG, "Error: BLE connection not allowed (provision_cb_thing_certificate)");
        return 0;
    }
    memset(provision_ble_buffer, 0, sizeof(provision_ble_buffer));
    memcpy(provision_ble_buffer,ctxt->om->om_data,ctxt->om->om_len);
    provision_ble_buffer_size = ctxt->om->om_len;

    event_trigger(EVENT_PROVISION_RECEIVE_THING_CERT);

    return 0;
}

static int provision_cb_thing_key(uint16_t handle_connection, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)handle_connection;
    (void)attr_handle;
    (void)arg;

    if (!ble_is_connection_allowed()){
        ESP_LOGE(TAG, "Error: BLE connection not allowed (provision_cb_thing_key)");
        return 0;
    }
    memset(provision_ble_buffer, 0, sizeof(provision_ble_buffer));
    memcpy(provision_ble_buffer,ctxt->om->om_data,ctxt->om->om_len);
    provision_ble_buffer_size = ctxt->om->om_len;

    event_trigger(EVENT_PROVISION_RECEIVE_THING_KEY);
    
    return 0;
}

static bool provision_set_pop(void)
{
    char decrypted_string[provision_ble_buffer_size - AES_BLOCK_SIZE + 1];
    if (!aes_crypto(provision_ble_buffer, decrypted_string, provision_ble_buffer_size)){
        ESP_LOGE(TAG, "Error: aes_crypto %s", provision_ble_buffer);
        return false;
    }

    char pop_str[BLE_POP_SIZE * 2 + 1];
    if (!ble_get_pop(pop_str)){
        ESP_LOGE(TAG, "Error: ble_get_pop");
    }

    ESP_LOGI(TAG, "POP NVS: %s", pop_str);
    ESP_LOGI(TAG, "POP BLE: %s", decrypted_string);
    if (strncmp(pop_str, decrypted_string, BLE_POP_SIZE * 2) != 0){
        ESP_LOGE(TAG, "Error: POP error");
        return false;
    }
    ESP_LOGI(TAG, "POP are equal");
    return true;
}

static bool provision_set_wifi_creds(void)
{
    if (provision_ble_buffer_size > BLE_BUFFER_SIZE){
        ESP_LOGE(TAG, "Error: Incoming BLE too large for buffer (provision_set_wifi_creds)");
        return false;
    }
    // The incoming data is a hex string where each byte is represented by two hexadecimal characters.
    // Therefore, the size of the actual byte data will be half the length of the hex string.
    // A char array is created to hold the decrypted data, its size is set to half of the hex string length.
    
    size_t provision_ble_buffer_byte_size = provision_ble_buffer_size;
    char decrypted_string[provision_ble_buffer_byte_size - AES_BLOCK_SIZE + 1];
    if (!aes_crypto(provision_ble_buffer, decrypted_string, provision_ble_buffer_byte_size)){
        ESP_LOGE(TAG, "Error: aes_crypto");
        return false;
    }

    cJSON *root = cJSON_Parse(decrypted_string);
    if (!root) {
        ESP_LOGE(TAG, "Error: parsing JSON input failed");
        cJSON_Delete(root);
        return false;
    }

    cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");

    if (!cJSON_IsString(type)) {
        ESP_LOGE(TAG, "Error: JSON type is not a string");
        cJSON_Delete(root);
        return false;
    }

    if (strcmp(type->valuestring, "wifi_credentials") == 0){
        ESP_LOGI(TAG, "Receiving WIFI creds");
        char *ssid = cJSON_GetObjectItemCaseSensitive(root, "ssid")->valuestring;
        char *password = cJSON_GetObjectItemCaseSensitive(root, "password")->valuestring;
        if (!wifi_set_ssid(ssid)){
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Error: wifi_set_ssid");
            return false;
        } 
        if (!wifi_set_pwd(password)){
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Error: wifi_set_pwd");
            return false;
        } 
        // Setting wifi credentials was successful
        cJSON_Delete(root);

        return true;
    }
    cJSON_Delete(root);
    ESP_LOGE(TAG, "Error: String is not wifi credentials");
    return false;
}

static blob_status_t provision_set_root_ca(void)
{
    if (provision_ble_buffer_size > BLE_BUFFER_SIZE){
        ESP_LOGE(TAG, "Error: Incoming BLE too large for buffer (provision_set_root_ca)");
        return BLOB_FAIL;
    }
    size_t provision_ble_buffer_byte_size = provision_ble_buffer_size;
    char decrypted_string[provision_ble_buffer_byte_size - AES_BLOCK_SIZE + 1];
    if (!aes_crypto(provision_ble_buffer, decrypted_string, provision_ble_buffer_byte_size)){
        ESP_LOGE(TAG, "Error: aes_crypto");
        return BLOB_FAIL;
    }

    cJSON *root = cJSON_Parse(decrypted_string);
    if (!root) {
        ESP_LOGE(TAG, "Error: parsing JSON input failed");
        cJSON_Delete(root);
        return BLOB_FAIL;
    }

    cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");

    if (!cJSON_IsString(type)) {
        ESP_LOGE(TAG, "Error: JSON type is not a string");
        cJSON_Delete(root);
        return BLOB_FAIL;
    }

    if (strcmp(type->valuestring, "aws_root_ca") == 0){
        ESP_LOGI(TAG, "Receiving AWS root CA");
        char *row = cJSON_GetObjectItemCaseSensitive(root, "row")->valuestring;
        int ready = cJSON_GetObjectItemCaseSensitive(root, "ready")->valueint;
        ESP_LOGI(TAG, "ready: %d, %s", ready, row);

        // Return fail if CA cert line didn't successfully write to NVS
        if (!auth_aws_provision_append_root_ca(row)) {
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Error: JSON type is not a string");
            return BLOB_FAIL;
        }
        // Return progress if ready is 0, as the last CA cert line hasn't been sent yet
        if (!ready){
            cJSON_Delete(root);
            return BLOB_PROGRESS;
        }
        // Return fail if "set CA flag" failed to write to NVS
        if (!auth_aws_provision_set_has_root_ca()) {
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Error: auth_aws_provision_set_has_root_ca");
            return BLOB_FAIL;
        }
        // Return done and successful
        cJSON_Delete(root);
        return BLOB_DONE;
    }
    cJSON_Delete(root);
    ESP_LOGE(TAG, "Error: String is not root CA");
    return BLOB_FAIL;
}

static blob_status_t provision_set_thing_cert(void)
{
    if (provision_ble_buffer_size > BLE_BUFFER_SIZE){
        ESP_LOGE(TAG, "Error: Incoming BLE too large for buffer (provision_set_thing_cert)");
        return BLOB_FAIL;
    }
    
    size_t provision_ble_buffer_byte_size = provision_ble_buffer_size;
    char decrypted_string[provision_ble_buffer_byte_size - AES_BLOCK_SIZE + 1];
    if (!aes_crypto(provision_ble_buffer, decrypted_string, provision_ble_buffer_byte_size)){
        ESP_LOGE(TAG, "Error: aes_crypto");
        return BLOB_FAIL;
    }

    cJSON *root = cJSON_Parse(decrypted_string);
    if (!root) {
        ESP_LOGE(TAG, "Error size %d: ", provision_ble_buffer_size);
        ESP_LOGE(TAG, "Error %s: ", decrypted_string);
        ESP_LOGE(TAG, "Error: parsing JSON input failed");
        cJSON_Delete(root);
        return BLOB_FAIL;
    }

    cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");

    if (!cJSON_IsString(type)) {
        ESP_LOGE(TAG, "Error: JSON type is not a string");
        cJSON_Delete(root);
        return BLOB_FAIL;
    }

    if (strcmp(type->valuestring, "aws_thing_certificate") == 0){
        ESP_LOGI(TAG, "Receiving AWS Thing certificate");
        char *row = cJSON_GetObjectItemCaseSensitive(root, "row")->valuestring;
        int ready = cJSON_GetObjectItemCaseSensitive(root, "ready")->valueint;
        ESP_LOGI(TAG, "ready: %d, %s", ready, row);

        // Return fail if CA cert line didn't successfully write to NVS
        if (!auth_aws_provision_append_thing_cert(row)) {
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Error: auth_aws_provision_append_thing_cert");
            return BLOB_FAIL;
        }
        if (!ready){
            cJSON_Delete(root);
            return BLOB_PROGRESS;
        }
        if (!auth_aws_provision_set_has_thing_cert()) {
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Error: auth_aws_provision_set_has_thing_cert");
            return BLOB_FAIL;
        }
        // Return done and successful
        cJSON_Delete(root);

        return BLOB_DONE;
    }
    cJSON_Delete(root);
    ESP_LOGE(TAG, "Error: String is not thing cert");
    return BLOB_FAIL;
}

static blob_status_t provision_set_thing_key(void)
{
    if (provision_ble_buffer_size > BLE_BUFFER_SIZE){
        ESP_LOGE(TAG, "Error: Incoming BLE too large for buffer (provision_set_thing_key)");
        return BLOB_FAIL;
    }

    size_t provision_ble_buffer_byte_size = provision_ble_buffer_size;
    char decrypted_string[provision_ble_buffer_byte_size - AES_BLOCK_SIZE + 1];
    if (!aes_crypto(provision_ble_buffer, decrypted_string, provision_ble_buffer_byte_size)){
        ESP_LOGE(TAG, "Error: aes_crypto");
        return BLOB_FAIL;
    }

    cJSON *root = cJSON_Parse(decrypted_string);
    if (!root) {
        ESP_LOGE(TAG, "Error: parsing JSON input failed");
        cJSON_Delete(root);
        return BLOB_FAIL;
    }

    cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");

    if (!cJSON_IsString(type)) {
        ESP_LOGE(TAG, "Error: JSON type is not a string");
        cJSON_Delete(root);
        return BLOB_FAIL;
    }

    if (strcmp(type->valuestring, "aws_thing_key") == 0){
        ESP_LOGI(TAG, "Receiving AWS Thing key");
        char *row = cJSON_GetObjectItemCaseSensitive(root, "row")->valuestring;
        int ready = cJSON_GetObjectItemCaseSensitive(root, "ready")->valueint;
        ESP_LOGI(TAG, "ready: %d, %s", ready, row);

        if (!auth_aws_provision_append_thing_key(row)) {
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Error: auth_aws_provision_append_thing_key");
            return BLOB_FAIL;
        }
        // Return progress if ready is 0, as the last CA cert line hasn't been sent yet
        if (!ready){
            cJSON_Delete(root);
            return BLOB_PROGRESS;
        }
        // Return fail if "set CA flag" failed to write to NVS
        if (!auth_aws_provision_set_has_thing_key()) {
            cJSON_Delete(root);
            ESP_LOGE(TAG, "Error: auth_aws_provision_set_has_thing_key");
            return BLOB_FAIL;
        }
        // Return done and successful
        cJSON_Delete(root);
        ESP_LOGI(TAG, "EVENT_PROVISIONING_DONE");
        return BLOB_DONE;
    }
    cJSON_Delete(root);
    ESP_LOGE(TAG, "Error: String is not thing key");
    return BLOB_FAIL;
}

static bool provision_create_network_message(wifi_ap_record_t ap_record, uint16_t count, char* buffer, size_t size)
{
    cJSON *message_root = cJSON_CreateObject();

    if (message_root == NULL) {
        ESP_LOGE(TAG, "Error: Message is NULL");
        return false;
    }

    cJSON_AddStringToObject(message_root, "type", "wifi_scan");
    cJSON_AddStringToObject(message_root, "ssid", (char *)ap_record.ssid);
    cJSON_AddNumberToObject(message_root, "rssi", ap_record.rssi);
    cJSON_AddNumberToObject(message_root, "count", count);

    // TODO maybe change to 0
    if (!cJSON_PrintPreallocated(message_root, buffer, size, 1)) {
        cJSON_Delete(message_root);
        ESP_LOGE(TAG, "Error: cJSON_PrintPreallocated");
        return false;
    }

    cJSON_Delete(message_root);
    return true;
}

static bool provision_create_status_message(prov_status_t status, char* buffer, size_t size)
{
    cJSON *message_root = cJSON_CreateObject();

    if (message_root == NULL) {
        ESP_LOGE(TAG, "Error: Message is NULL");
        return false;
    }

    cJSON_AddStringToObject(message_root, "type", "provision");
    if (status == PROV_DONE){
        cJSON_AddStringToObject(message_root, "status", "done");
    } 
    if (status == PROV_PROGRESS) {
        cJSON_AddStringToObject(message_root, "status", "progress");
    }
    if (status == PROV_FAIL) {
        cJSON_AddStringToObject(message_root, "status", "fail");
    }
    if (!cJSON_PrintPreallocated(message_root, buffer, size, 1)) {
        cJSON_Delete(message_root);
        ESP_LOGE(TAG, "Error: cJSON_PrintPreallocated");
        return false;
    }
    cJSON_Delete(message_root);
    return true;
}

static bool provision_notify_wifi_scan(void)
{
    if (!ble_is_connection_allowed()){
        ESP_LOGE(TAG, "Error: BLE connection not allowed (provision_notify_wifi_scan)");
        return false;
    }
    uint16_t list_max_size = WIFI_MAX_SCAN_RESULTS;
    wifi_ap_record_t network_ssid_list[WIFI_MAX_SCAN_RESULTS];
    if (!wifi_get_network_list(network_ssid_list, &list_max_size)){
        ESP_LOGE(TAG, "Error: wifi_get_network_list");
        return false;
    }

    for(uint16_t i = 0; i < list_max_size; i++) {
        if (!provision_create_network_message(  network_ssid_list[i], 
                                                list_max_size - i - 1, 
                                                provision_ble_buffer, 
                                                BLE_BUFFER_SIZE)){
            ESP_LOGE(TAG, "Error: provision_create_network_message");
            return false;
        }
        if (!ble_send_notification(handle_notification, provision_ble_buffer)){
            ESP_LOGE(TAG, "Error: ble_send_notification");
            return false;
        }
    }

    event_trigger(EVENT_PROVISION_NOTIFYING_WIFI_SCAN);

    return true;
}

static bool provision_notify_status(prov_status_t status)
{
    if (!ble_is_connection_allowed()){
        ESP_LOGE(TAG, "Error: BLE connection not allowed (provision_notify_status)");
        return false;
    }
    if (!provision_create_status_message(status, provision_ble_buffer, BLE_BUFFER_SIZE)){
        ESP_LOGE(TAG, "Error: provision_create_status_message");
        return false;
    }

    event_trigger(EVENT_PROVISION_NOTIFYING_STATUS);
    if (!ble_send_notification(handle_notification, provision_ble_buffer)){
        ESP_LOGE(TAG, "Error: ble_send_notification");
        return false;
    }
    return true;
}

static bool provision_register_ble_service(void) 
{
    if (!ble_register_service(provision_service[0])){
        ESP_LOGE(TAG, "Error: ble_register_service");
        return false;
    }
    return true;
}

bool provision_run(void)
{
    switch(event_wait()){
        case EVENT_BLE_GAP_CONNECTED:
            event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                            EVENT_PROVISION_RECEIVE_POP);
            ble_set_allow_connection(true);
            mqtt_stop();
            break;
        case EVENT_PROVISION_RECEIVE_POP:
            if (!provision_set_pop()){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_NOTIFYING_STATUS);
                provision_notify_status(PROV_FAIL);
                ble_set_allow_connection(false);
            } else {
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_WIFI_SCAN_DONE);
                // Delete the current secrets
                wifi_erase_credentials();
                auth_aws_provision_erase_root_ca();
                auth_aws_provision_erase_thing_cert();
                auth_aws_provision_erase_thing_key();
                ble_set_allow_connection(true);
                provision_notify_status(PROV_PROGRESS);
                // Do a scan of available wifi networks
                wifi_start_scan();
            }
            break;
        case EVENT_WIFI_SCAN_DONE:
            // Send found wifi networks to mobile app over BLE
            event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                            EVENT_PROVISION_NOTIFYING_WIFI_SCAN);
            if (!provision_notify_wifi_scan()){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_NOTIFYING_STATUS);
                provision_notify_status(PROV_FAIL);
                ble_set_allow_connection(false);
            }
            break;
        case EVENT_PROVISION_NOTIFYING_WIFI_SCAN:
            event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                            EVENT_PROVISION_RECEIVE_WIFI_CREDS);
            break;
        case EVENT_PROVISION_RECEIVE_WIFI_CREDS:
            event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                            EVENT_PROVISION_RECEIVE_ROOT_CA);
            if (!provision_set_wifi_creds()){
                provision_notify_status(PROV_FAIL);
                ble_set_allow_connection(false);
            } else {
                provision_notify_status(PROV_PROGRESS);
            }
            break;
        case EVENT_PROVISION_RECEIVE_ROOT_CA:
            blob_status_t status_ca_root = provision_set_root_ca();
            if (status_ca_root == BLOB_PROGRESS){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_RECEIVE_ROOT_CA);
                provision_notify_status(PROV_PROGRESS);
            }
            if (status_ca_root == BLOB_FAIL){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_NOTIFYING_STATUS);
                provision_notify_status(PROV_FAIL);
                ble_set_allow_connection(false);
            }
            if (status_ca_root == BLOB_DONE){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_RECEIVE_THING_CERT);
                provision_notify_status(PROV_PROGRESS);
            }
            break;
        case EVENT_PROVISION_RECEIVE_THING_CERT:
            blob_status_t status_thing_cert = provision_set_thing_cert();
            if (status_thing_cert == BLOB_PROGRESS){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_RECEIVE_THING_CERT);
                provision_notify_status(PROV_PROGRESS);
            }
            if (status_thing_cert == BLOB_FAIL){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_NOTIFYING_STATUS);
                provision_notify_status(PROV_FAIL);
                ble_set_allow_connection(false);
            }
            if (status_thing_cert == BLOB_DONE){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_RECEIVE_THING_KEY);
                provision_notify_status(PROV_PROGRESS);
            }
            break;
        case EVENT_PROVISION_RECEIVE_THING_KEY:
            blob_status_t status_thing_key = provision_set_thing_key();
            if (status_thing_key == BLOB_PROGRESS){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_RECEIVE_THING_KEY);
                provision_notify_status(PROV_PROGRESS);
            }
            if (status_thing_key == BLOB_FAIL){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_NOTIFYING_STATUS);
                provision_notify_status(PROV_FAIL);
                ble_set_allow_connection(false);
            }
            if (status_thing_key == BLOB_DONE){
                if (!wifi_reinit()){
                    event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                    EVENT_PROVISION_NOTIFYING_STATUS);
                    provision_notify_status(PROV_FAIL);
                    ble_set_allow_connection(false);
                } else {
                    event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                    EVENT_WIFI_START);
                    provision_notify_status(PROV_PROGRESS);
                }
            }
            break;
        case EVENT_WIFI_START:
            event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                            EVENT_WIFI_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED);
            if (!wifi_connect()){
                event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                                EVENT_PROVISION_NOTIFYING_STATUS);
                provision_notify_status(PROV_FAIL);
                ble_set_allow_connection(false); 
            }
            break;
        case EVENT_WIFI_CONNECTED:
            event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                            EVENT_PROVISION_NOTIFYING_STATUS);
            auth_use_provision();
            provision_notify_status(PROV_DONE);
            break;
        case EVENT_WIFI_DISCONNECTED:
            event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                            EVENT_PROVISION_NOTIFYING_STATUS);
            // Notify mobile app that provisioning failed, probably due to wrong wifi password
            provision_notify_status(PROV_FAIL);
            break;
        case EVENT_PROVISION_NOTIFYING_STATUS:
            event_expect(   EVENT_BLE_GAP_DISCONNECTED | 
                            EVENT_BLE_NOTIFY_DONE);
            break;
        case EVENT_BLE_NOTIFY_DONE:
            // Return false to reboot
            return false;
            break;
        case EVENT_BLE_GAP_DISCONNECTED:
            // Return false to reboot
            return false;
            break;
        case EVENT_IGNORE:
            break;
        default:
            break;
    }

    return true;
}

bool provision_init(void)
{
    ESP_LOGI(TAG, "Initialise");

    if (!provision_register_ble_service()){
        ESP_LOGE(TAG, "Error: provision_register_ble_service");
        return false;
    }
    return true;
}