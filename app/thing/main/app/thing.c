#include "app/thing.h"
#include <cJSON.h>
#include <string.h>
#include "esp_log.h"
#include "utilities/misc.h"
#include "drivers/storage.h"
#include "utilities/event.h"
#include "utilities/state.h"
#include "middlewares/mqtt.h"
#include "middlewares/wifi.h"
#include "app/mobile.h"
#include "utilities/auth_aws_provision.h"
#include "middlewares/ble.h"
#include "app/ota.h"
#include "app/types/type.h"

#define THING_STORAGE_KEY_TYPE          "thing_type"
#define THING_STORAGE_KEY_HW_VERSION    "thing_hw"
#define THING_STORAGE_KEY_FLAGS         "thing_flags"

#define MQTT_TOPIC_PUB_BASE             "thingpub/"
#define MQTT_TOPIC_SUB_BASE             "thingsub/"
#define MQTT_TOPIC_ACTION_OTAURL        "/otaurl"
#define MQTT_TOPIC_ACTION_VALUE         "/value"
#define MQTT_TOPIC_ACTION_BOOTUP        "/bootup"

#define THING_OTA_SCHEDULE_SECONDS      (24 * 60 * 60 * 1000) // 24h


static const char *TAG = "THING";

static char thing_type[THING_TYPE_NAME_MAX_SIZE];
static char thing_hw_version[THING_HW_VERSION_MAX_SIZE];

static char thing_mqtt_topic_pub_otaurl[MQTT_TOPIC_MAX_SIZE];
static char thing_mqtt_topic_sub_otaurl[MQTT_TOPIC_MAX_SIZE];
static char thing_mqtt_topic_pub_value[MQTT_TOPIC_MAX_SIZE];
static char thing_mqtt_topic_sub_value[MQTT_TOPIC_MAX_SIZE];
static char thing_mqtt_topic_pub_bootup[MQTT_TOPIC_MAX_SIZE];
static char thing_mqtt_topic_sub_bootup[MQTT_TOPIC_MAX_SIZE];

static char thing_mqtt_data_buffer[MQTT_DATA_MAX_LEN];

static SemaphoreHandle_t thing_mqtt_data_buffer_lock = NULL;

static bool thing_set_value(void);
static bool thing_get_value(void);

static void thing_mqtt_received_value_cb(const char *data, int data_len);
static void thing_mqtt_received_otaurl_cb(const char *data, int data_len);
static void thing_mqtt_received_bootup_cb(const char *data, int data_len);

static void thing_mqtt_publish_value_cb(void);

static bool thing_create_mqtt_topic_pub_otaurl(void);
static bool thing_create_mqtt_topic_sub_otaurl(void);
static bool thing_create_mqtt_topic_pub_value(void);
static bool thing_create_mqtt_topic_sub_value(void);
static bool thing_create_mqtt_topic_pub_bootup(void);
static bool thing_create_mqtt_topic_sub_bootup(void);


static bool thing_take_mqtt_buffer_lock(void);
static void thing_give_mqtt_buffer_lock(void);

static bool thing_publish_otaurl(void);
static bool thing_publish_value(void);
static bool thing_publish_bootup(void);


static void thing_schedule_ota_task(void* arg);

static bool thing_set_has_type(void);
static bool thing_load_type(void);
static bool thing_set_has_hw_version(void);
static bool thing_load_hw_version(void);


static bool thing_create_mqtt_topic_pub_otaurl(void)
{
    char thing_id[ID_SIZE];
    if (!id_get(thing_id)){
        ESP_LOGE(TAG, "Error: id_get");
        return false;
    }

    size_t topic_size = strlen(MQTT_TOPIC_PUB_BASE) + strlen(thing_id) + strlen(MQTT_TOPIC_ACTION_OTAURL) + 1;
    
    if (topic_size > MQTT_TOPIC_MAX_SIZE) {
        ESP_LOGE(TAG, "Error: Topic size is too large");
        return false;
    }
    
    int ret = snprintf(thing_mqtt_topic_pub_otaurl, topic_size, "%s%s%s", MQTT_TOPIC_PUB_BASE, thing_id, MQTT_TOPIC_ACTION_OTAURL);
    if (ret < 0 || ret > topic_size) {
        ESP_LOGE(TAG, "Error: snprintf");
        return false;
    }
    return true;
}

static bool thing_create_mqtt_topic_pub_value(void)
{
    char thing_id[ID_SIZE];
    if (!id_get(thing_id)){
        ESP_LOGE(TAG, "Error: id_get");
        return false;
    }

    size_t topic_size = strlen(MQTT_TOPIC_PUB_BASE) + strlen(thing_id) + strlen(MQTT_TOPIC_ACTION_VALUE) + 1;
    
    if (topic_size > MQTT_TOPIC_MAX_SIZE) {
        ESP_LOGE(TAG, "Error: Topic size is too large");
        return false;
    }

    int ret = snprintf(thing_mqtt_topic_pub_value, topic_size, "%s%s%s", MQTT_TOPIC_PUB_BASE, thing_id, MQTT_TOPIC_ACTION_VALUE);
    if (ret < 0 || ret > topic_size) {
        ESP_LOGE(TAG, "Error: snprintf");
        return false;
    }
    return true;
}

static bool thing_create_mqtt_topic_pub_bootup(void)
{
    char thing_id[ID_SIZE];
    if (!id_get(thing_id)){
        ESP_LOGE(TAG, "Error: id_get");
        return false;
    }

    size_t topic_size = strlen(MQTT_TOPIC_PUB_BASE) + strlen(thing_id) + strlen(MQTT_TOPIC_ACTION_BOOTUP) + 1;
    
    if (topic_size > MQTT_TOPIC_MAX_SIZE) {
        ESP_LOGE(TAG, "Error: Topic size is too large");
        return false;
    }

    int ret = snprintf(thing_mqtt_topic_pub_bootup, topic_size, "%s%s%s", MQTT_TOPIC_PUB_BASE, thing_id, MQTT_TOPIC_ACTION_BOOTUP);
    if (ret < 0 || ret > topic_size) {
        ESP_LOGE(TAG, "Error: snprintf");
        return false;
    }
    return true;
}

static bool thing_create_mqtt_topic_sub_otaurl(void)
{
    char thing_id[ID_SIZE];
    if (!id_get(thing_id)){
        ESP_LOGE(TAG, "Error: id_get");
        return false;
    }

    size_t topic_size = strlen(MQTT_TOPIC_SUB_BASE) + strlen(thing_id) + strlen(MQTT_TOPIC_ACTION_OTAURL) + 1;
    
    if (topic_size > MQTT_TOPIC_MAX_SIZE) {
        ESP_LOGE(TAG, "Error: Topic size is too large");
        return false;
    }
    int ret = snprintf(thing_mqtt_topic_sub_otaurl, topic_size, "%s%s%s", MQTT_TOPIC_SUB_BASE, thing_id, MQTT_TOPIC_ACTION_OTAURL);
    if (ret < 0 || ret > topic_size) {
        ESP_LOGE(TAG, "Error: snprintf");
        return false;
    }

    return true;
}

static bool thing_create_mqtt_topic_sub_value(void)
{
    char thing_id[ID_SIZE];
    if (!id_get(thing_id)){
        ESP_LOGE(TAG, "Error: id_get");
        return false;
    }

    size_t topic_size = strlen(MQTT_TOPIC_SUB_BASE) + strlen(thing_id) + strlen(MQTT_TOPIC_ACTION_VALUE) + 1;
    
    if (topic_size > MQTT_TOPIC_MAX_SIZE) {
        ESP_LOGE(TAG, "Error: Topic size is too large");
        return false;
    }

    int ret = snprintf(thing_mqtt_topic_sub_value, topic_size, "%s%s%s", MQTT_TOPIC_SUB_BASE, thing_id, MQTT_TOPIC_ACTION_VALUE);
    if (ret < 0 || ret > topic_size) {
        ESP_LOGE(TAG, "Error: snprintf");
        return false;
    }

    return true;
}

static bool thing_create_mqtt_topic_sub_bootup(void)
{
    char thing_id[ID_SIZE];
    if (!id_get(thing_id)){
        ESP_LOGE(TAG, "Error: id_get");
        return false;
    }

    size_t topic_size = strlen(MQTT_TOPIC_SUB_BASE) + strlen(thing_id) + strlen(MQTT_TOPIC_ACTION_BOOTUP) + 1;
    
    if (topic_size > MQTT_TOPIC_MAX_SIZE) {
        ESP_LOGE(TAG, "Error: Topic size is too large");
        return false;
    }

    int ret = snprintf(thing_mqtt_topic_sub_bootup, topic_size, "%s%s%s", MQTT_TOPIC_SUB_BASE, thing_id, MQTT_TOPIC_ACTION_BOOTUP);
    if (ret < 0 || ret > topic_size) {
        ESP_LOGE(TAG, "Error: snprintf");
        return false;
    }

    return true;
}

static bool thing_set_otaurl(void)
{
    if (!ota_set_url(thing_mqtt_data_buffer)){
        thing_give_mqtt_buffer_lock();
        ESP_LOGI(TAG, "Do not set OTA URL");
        return false;
    }
    thing_give_mqtt_buffer_lock();
    return true;
}

static bool thing_set_value(void)
{
    cJSON *value = cJSON_Parse(thing_mqtt_data_buffer);
    
    if(!value) {
        ESP_LOGE(TAG, "Error: Value is null");
        return false;
    }

    if (!mobile_set_value_json(thing_mqtt_data_buffer)){
        thing_give_mqtt_buffer_lock();
        ESP_LOGE(TAG, "Error: mobile_set_value_json");
        return false;
    }

    cJSON *thing_value = cJSON_GetObjectItem(value, "thing_value");
    if (!cJSON_IsObject(thing_value)) {
        cJSON_Delete(value);
        thing_give_mqtt_buffer_lock();
        ESP_LOGE(TAG, "Error: cJSON_IsObject");
        return false;
    }

    if (!type_set_value_json(thing_value)){
        cJSON_Delete(value);
        thing_give_mqtt_buffer_lock();
        ESP_LOGE(TAG, "Error: type_set_value_json");
        return false;
    }

    // value is not needed anymore, we can delete it to free up memory
    cJSON_Delete(value);
    value = NULL;

    thing_give_mqtt_buffer_lock();
    return true;
}


static bool thing_get_value(void)
{
    cJSON *thing_value = cJSON_CreateObject();
    cJSON *mobile_value = cJSON_CreateObject();
    cJSON *value = cJSON_CreateObject();
    cJSON *root = cJSON_CreateObject();

    if (!thing_value || !mobile_value || !value || !root) {
        ESP_LOGE(TAG, "Error: !thing_value || !mobile_value || !value || !root");
        goto error;
    }

    cJSON *readwrite = cJSON_CreateObject();
    cJSON *read = cJSON_CreateObject();
    if (!readwrite || !read || 
        !cJSON_AddItemToObject(thing_value, "readwrite", readwrite) ||
        !cJSON_AddItemToObject(thing_value, "read", read)) {
        ESP_LOGE(TAG, "Error: !readwrite || !read");
        goto error;
    }

    if (!type_get_value_json(thing_value)){
        ESP_LOGE(TAG, "Error: type_get_value_json");
        goto error;
    }

    cJSON_AddStringToObject(read, "hw_version", thing_hw_version);
    cJSON_AddStringToObject(read, "fw_version", PROJECT_VER);

    if (!mobile_get_value_json(mobile_value)) {
        ESP_LOGE(TAG, "Error: mobile_get_value_json");
        goto error;
    }

    if (!cJSON_AddItemToObject(value, "thing_value", thing_value) ||
        !cJSON_AddItemToObject(value, "mobile_value", mobile_value)) {
        ESP_LOGE(TAG, "Error: cJSON_AddItemToObject");
        goto error;
    }

    if (!cJSON_PrintPreallocated(value, thing_mqtt_data_buffer, sizeof(thing_mqtt_data_buffer), 0)) {
        ESP_LOGE(TAG, "Error: cJSON_PrintPreallocated value");
        goto error;
    }

    cJSON_AddStringToObject(root, "value", thing_mqtt_data_buffer);

    if (!cJSON_PrintPreallocated(root, thing_mqtt_data_buffer, sizeof(thing_mqtt_data_buffer), 0)) {
        ESP_LOGE(TAG, "Error: cJSON_PrintPreallocated root");
        goto error;
    }

    cJSON_Delete(root);
    cJSON_Delete(value);

    return true;

error:
    cJSON_Delete(thing_value);
    cJSON_Delete(mobile_value);
    cJSON_Delete(value);
    cJSON_Delete(root);
    ESP_LOGE(TAG, "Error: goto error");
    return false;
}

static void thing_schedule_ota_task(void* arg) 
{
    while(true) {
        vTaskDelay(THING_OTA_SCHEDULE_SECONDS / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "thing_mqtt_publish_otaurl_cb triggered!");
        event_trigger(EVENT_THING_PUBLISH_OTAURL);
    }
}

static void thing_mqtt_publish_value_cb(void) 
{
    ESP_LOGI(TAG, "thing_mqtt_publish_value_cb triggered!");
    event_trigger(EVENT_THING_PUBLISH_VALUE);
}

static void thing_mqtt_received_value_cb(const char *data, int data_len)
{
    if(!thing_take_mqtt_buffer_lock()){
        ESP_LOGE(TAG, "Error: thing_take_mqtt_buffer_lock");
        return;
    }
    if (data_len > sizeof(thing_mqtt_data_buffer)){
        ESP_LOGI(TAG, "Error: MQTT data too long for buffer");
        return;
    }
    memcpy(thing_mqtt_data_buffer, data, data_len);

    ESP_LOGI(TAG, "thing_mqtt_received_value_cb triggered!");
    event_trigger(EVENT_THING_RECEIVED_VALUE);
}

static void thing_mqtt_received_otaurl_cb(const char *data, int data_len)
{
    if(!thing_take_mqtt_buffer_lock()){
        ESP_LOGI(TAG, "Error: thing_take_mqtt_buffer_lock");
        return;
    }
    if (data_len > sizeof(thing_mqtt_data_buffer)){
        ESP_LOGI(TAG, "Error: MQTT data too long for buffer");
        return;
    }
    memcpy(thing_mqtt_data_buffer, data, data_len);

    ESP_LOGI(TAG, "thing_mqtt_received_otaurl_cb triggered!");
    event_trigger(EVENT_THING_RECEIVED_OTAURL);
}

static void thing_mqtt_received_bootup_cb(const char *data, int data_len)
{
    if(!thing_take_mqtt_buffer_lock()){
        ESP_LOGE(TAG, "Error: thing_take_mqtt_buffer_lock");
        return;
    }
    if (data_len > sizeof(thing_mqtt_data_buffer)){
        ESP_LOGI(TAG, "Error: MQTT data too long for buffer");
        return;
    }
    memcpy(thing_mqtt_data_buffer, data, data_len);

    ESP_LOGI(TAG, "thing_mqtt_received_bootup_cb triggered!");
    event_trigger(EVENT_THING_RECEIVED_BOOTUP);
}

static bool thing_publish_otaurl(void)
{
    if (!thing_take_mqtt_buffer_lock()){
        ESP_LOGE(TAG, "Error: thing_take_mqtt_buffer_lock");
        return false;
    }
    if (!thing_get_value()){
        thing_give_mqtt_buffer_lock();
        ESP_LOGE(TAG, "Error: thing_get_value");
        return false;
    }
    if (!mqtt_publish(thing_mqtt_topic_pub_otaurl, thing_mqtt_data_buffer)){
        thing_give_mqtt_buffer_lock();
        ESP_LOGE(TAG, "Error: mqtt_publish");
        return false;
    }
    thing_give_mqtt_buffer_lock();
    return true;
}

static bool thing_publish_bootup(void)
{
    return mqtt_publish(thing_mqtt_topic_pub_bootup, "{}");
}

static bool thing_publish_value(void)
{
    if (!thing_take_mqtt_buffer_lock()){
        ESP_LOGE(TAG, "Error: thing_take_mqtt_buffer_lock");
        return false;
    }
    if (!thing_get_value()){
        thing_give_mqtt_buffer_lock();
        ESP_LOGE(TAG, "Error: thing_get_value");
        return false;
    }
    if (!mqtt_publish(thing_mqtt_topic_pub_value, thing_mqtt_data_buffer)){
        thing_give_mqtt_buffer_lock();
        ESP_LOGE(TAG, "Error: mqtt_publish");
        return false;
    }
    thing_give_mqtt_buffer_lock();
    return true;
}

static bool thing_take_mqtt_buffer_lock(void)
{
    if (!(xSemaphoreTake(thing_mqtt_data_buffer_lock, 0) == pdTRUE)){
        ESP_LOGE(TAG, "Error: xSemaphoreTake");
        return false;
    }
    memset(thing_mqtt_data_buffer, 0, sizeof(thing_mqtt_data_buffer));
    return true;
}

static void thing_give_mqtt_buffer_lock(void)
{
    xSemaphoreGive(thing_mqtt_data_buffer_lock);
}

static bool thing_set_has_type(void)
{
    return storage_set_flags(THING_STORAGE_KEY_FLAGS, THING_HAS_TYPE);
}

static bool thing_load_type(void)
{
    if (!thing_get_has_type()){
        ESP_LOGE(TAG, "Error: thing_get_has_type");
        return false;
    }
    return storage_get_blob(THING_STORAGE_KEY_TYPE, thing_type, THING_TYPE_NAME_MAX_SIZE);
}

bool thing_get_has_type(void)
{
    return storage_has_flags(THING_STORAGE_KEY_FLAGS, THING_HAS_TYPE);
}

bool thing_set_type(const char* type)
{
    if (strlen(type) + 1 > THING_TYPE_NAME_MAX_SIZE){
        ESP_LOGE(TAG, "Error: Type is too long for buffer");
        return false;
    }
    if (thing_get_has_type()){
        ESP_LOGE(TAG, "Error: thing_get_has_type");
        return false;
    }
    if (!storage_set_blob(THING_STORAGE_KEY_TYPE, type, strlen(type) + 1)){
        ESP_LOGE(TAG, "Error: storage_set_blob");
        return false;
    }
    return thing_set_has_type();
}

static bool thing_set_has_hw_version(void)
{
    return storage_set_flags(THING_STORAGE_KEY_FLAGS, THING_HAS_HW_VERSION);
}

static bool thing_load_hw_version(void)
{
    if (!thing_get_has_hw_version()){
        ESP_LOGE(TAG, "Error: thing_get_has_hw_version");
        return false;
    }
    return storage_get_blob(THING_STORAGE_KEY_HW_VERSION, thing_hw_version, THING_HW_VERSION_MAX_SIZE);
}

bool thing_get_has_hw_version(void)
{
    return storage_has_flags(THING_STORAGE_KEY_FLAGS, THING_HAS_HW_VERSION);
}

bool thing_set_hw_version(const char* hw_version)
{
    if (strlen(hw_version) + 1> THING_HW_VERSION_MAX_SIZE){
        ESP_LOGE(TAG, "Error: hw_version is too large for buffer");
        return false;
    }
    if (thing_get_has_hw_version()){
        ESP_LOGE(TAG, "Error: thing_get_has_hw_version");
        return false;
    }
    if (!storage_set_blob(THING_STORAGE_KEY_HW_VERSION, hw_version, strlen(hw_version) + 1)){
        ESP_LOGE(TAG, "Error: storage_set_blob");
        return false;
    }
    return thing_set_has_hw_version();
}

bool thing_reboot(void)
{
    ESP_LOGW(TAG, "Pre-reboot");

    type_pre_reboot();

    ESP_LOGI(TAG, "Reboot...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
}

bool thing_init(void)
{
    ESP_LOGI(TAG, "Initialise");
    if (!thing_load_type()){
        ESP_LOGE(TAG, "Error: thing_load_type");
        return false;
    }
    if (!thing_load_hw_version()){
        ESP_LOGE(TAG, "Error: thing_load_hw_version");
        return false;
    }
    if (!type_set_int(thing_type)){
        ESP_LOGE(TAG, "Error: type_set_int, using Default type");
        // Don't return false as default type can be used
    }
    if (!type_init(thing_mqtt_publish_value_cb)){
        ESP_LOGE(TAG, "Error: type_init");
        return false;
    }
    if (!mobile_init()){
        ESP_LOGE(TAG, "Error: mobile_init");
        return false;
    }
    if (!thing_create_mqtt_topic_pub_otaurl()){
        ESP_LOGE(TAG, "Error: thing_create_mqtt_topic_pub_otaurl");
        return false;
    }
    if (!thing_create_mqtt_topic_sub_otaurl()){
        ESP_LOGE(TAG, "Error: thing_create_mqtt_topic_sub_otaurl");
        return false;
    }
    if (!thing_create_mqtt_topic_pub_value()){
        ESP_LOGE(TAG, "Error: thing_create_mqtt_topic_pub_value");
        return false;
    }
    if (!thing_create_mqtt_topic_sub_value()){
        ESP_LOGE(TAG, "Error: thing_create_mqtt_topic_sub_value");
        return false;
    }
    if (!thing_create_mqtt_topic_pub_bootup()){
        ESP_LOGE(TAG, "Error: thing_create_mqtt_topic_pub_bootup");
        return false;
    }
    if (!thing_create_mqtt_topic_sub_bootup()){
        ESP_LOGE(TAG, "Error: thing_create_mqtt_topic_sub_bootup");
        return false;
    }
    if (!mqtt_register_subscription(thing_mqtt_topic_sub_otaurl, thing_mqtt_received_otaurl_cb)){
        ESP_LOGE(TAG, "Error: mqtt_register_subscription");
        return false;
    }
    if (!mqtt_register_subscription(thing_mqtt_topic_sub_value, thing_mqtt_received_value_cb)){
        ESP_LOGE(TAG, "Error: mqtt_register_subscription");
        return false;
    }
    if (!mqtt_register_subscription(thing_mqtt_topic_sub_bootup, thing_mqtt_received_bootup_cb)){
        ESP_LOGE(TAG, "Error: mqtt_register_subscription");
        return false;
    }
    if (xTaskCreate(thing_schedule_ota_task, "thing_schedule_ota_task", 2048, NULL, 5, NULL) != pdTRUE){
        ESP_LOGE(TAG, "Error: thing_schedule_ota_task");
        return false;
    }

    thing_mqtt_data_buffer_lock = xSemaphoreCreateBinary();
    thing_give_mqtt_buffer_lock();
    
    return true;
}

bool thing_run(void)
{
    switch(event_wait()){
        case EVENT_BLE_GAP_CONNECTED:
            state_set(STATE_PROVISION);
            event_expect(EVENT_BLE_GAP_DISCONNECTED | EVENT_PROVISION_RECEIVE_POP);
            ble_set_allow_connection(true);
            mqtt_stop();
            break;
        case EVENT_WIFI_START:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED);
            wifi_connect();
            break;
        case EVENT_WIFI_CONNECTED:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED | 
                            EVENT_MQTT_CONNECTED);
            break;
        case EVENT_WIFI_DISCONNECTED:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED);
            wifi_connect();
            break;
        case EVENT_MQTT_CONNECTED:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED | 
                            EVENT_MQTT_SUBSCRIBED);
            mqtt_subscribe();
            break;
        case EVENT_MQTT_SUBSCRIBED:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED | 
                            EVENT_THING_RECEIVED_BOOTUP);
            thing_publish_bootup();
            break;
        case EVENT_THING_RECEIVED_BOOTUP:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED | 
                            EVENT_THING_RECEIVED_OTAURL);
            thing_set_value();
            thing_publish_otaurl();
            break;
        case EVENT_THING_RECEIVED_OTAURL:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED | 
                            EVENT_THING_RECEIVED_VALUE |
                            EVENT_THING_PUBLISH_OTAURL |
                            EVENT_THING_PUBLISH_VALUE);
            if (thing_set_otaurl()){
                state_set(STATE_OTA);
            } else {
                thing_publish_value();
            }
            break;
        case EVENT_THING_RECEIVED_VALUE:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED | 
                            EVENT_THING_RECEIVED_VALUE |
                            EVENT_THING_PUBLISH_OTAURL |
                            EVENT_THING_PUBLISH_VALUE);
            thing_set_value();
            thing_publish_value();
            break;
        case EVENT_THING_PUBLISH_OTAURL:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED | 
                            EVENT_THING_RECEIVED_OTAURL);
            thing_publish_otaurl();
            break;
        case EVENT_THING_PUBLISH_VALUE:
            event_expect(   EVENT_BLE_GAP_CONNECTED | 
                            EVENT_WIFI_DISCONNECTED | 
                            EVENT_MQTT_SUBSCRIBED |
                            EVENT_THING_RECEIVED_VALUE |
                            EVENT_THING_PUBLISH_VALUE);
            thing_publish_value();
            break;
        case EVENT_IGNORE:
            break;
        default:
            return false;
    }

    return true;
}
