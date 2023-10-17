/*
 * mqtt.c
 *
 *  Created on: 25 may 2023
 *      Author: klaslofstedt
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <sys/param.h>
#include "middlewares/mqtt.h"
#include "utilities/misc.h"
#include "utilities/event.h"
#include "drivers/storage.h"
#include "aws_mqtt_endpoint.h"
#include "utilities/auth_aws_ota.h"
#include "utilities/auth_aws_provision.h"
#include "middlewares/auth.h"


#define MQTT_MAX_TOPICS 3

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t mqtt_client = NULL;
static const char* mqtt_topic_array[MQTT_MAX_TOPICS] = {NULL, NULL, NULL};
static mqtt_received_callback_t mqtt_received_callbacks[MQTT_MAX_TOPICS] = {NULL, NULL, NULL};
static bool mqtt_is_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    // TODO: This seems a bit shit
    mqtt_client = client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            event_trigger(EVENT_MQTT_CONNECTED);
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_is_connected = false;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            mqtt_is_connected = true;
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            event_trigger(EVENT_MQTT_SUBSCRIBED);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            mqtt_is_connected = false;
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED: // Will not be triggered with QoS0 (fire&forget)
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "DATA LEN: %d, TOPIC LEN: %d", event->data_len, event->topic_len);
            ESP_LOGI(TAG, "Received on topic - %.*s, data - %.*s", event->topic_len, event->topic, event->data_len, event->data);
            event_trigger(EVENT_MQTT_DATA_RECEIVED);

            for (int i = 0; i < MQTT_MAX_TOPICS; i++) {
                if (mqtt_topic_array[i] != NULL && strncmp(event->topic, mqtt_topic_array[i], event->topic_len) == 0) {
                    if (mqtt_received_callbacks[i]) {
                        mqtt_received_callbacks[i](event->data, event->data_len);
                    }
                    break;
                }
            }            
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(TAG, "Error: Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGE(TAG, "Error: Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                ESP_LOGE(TAG, "Error: Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                        strerror(event->error_handle->esp_transport_sock_errno));
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGE(TAG, "Error: Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGE(TAG, "Error: Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

bool mqtt_register_subscription(const char* topic, mqtt_received_callback_t received_callback) 
{
    for (int i = 0; i < MQTT_MAX_TOPICS; i++) {
        if (mqtt_topic_array[i] == NULL) {
            mqtt_topic_array[i] = topic;
            mqtt_received_callbacks[i] = received_callback;
            return true;
        }
    }
    ESP_LOGE(TAG, "Error: No topic slots available");
    return false;
}

bool mqtt_subscribe(void) 
{
    for (int i = 0; i < MQTT_MAX_TOPICS; i++) {
        if (mqtt_topic_array[i] != NULL) {
            int result = esp_mqtt_client_subscribe(mqtt_client, mqtt_topic_array[i], 0);
            if (result < 0) {
                ESP_LOGE(TAG, "Error: Failed to subscribe");
                return false;
            }
        }
    }
    return true;
}

bool mqtt_publish(const char* topic, const char* data)
{
    if (!mqtt_is_connected){
        ESP_LOGE(TAG, "Error: MQTT not connected");
        return false;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, data, strlen(data), 0, 0);

    if (msg_id < 0){
        ESP_LOGI(TAG, "Error: Publishing failed");
        return false;
    } 
    ESP_LOGI(TAG, "Published msg_id: %d to %s", msg_id, topic);
    ESP_LOGI(TAG, "Published message: %s", data);
    return true;
}

bool mqtt_stop(void)
{
    ESP_LOGI(TAG, "Deinitialise");

    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "MQTT: Client is NULL");
        return false;
    }

    esp_err_t err = esp_mqtt_client_stop(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Failed to stop MQTT client");
        return false;
    }

    err = esp_mqtt_client_destroy(mqtt_client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Failed to destroy MQTT client");
        return false;
    }

    mqtt_client = NULL;

    return true;
}

bool mqtt_init(void)
{
    ESP_LOGI(TAG, "Initialise");

    esp_mqtt_client_config_t mqtt_config;
    memset(&mqtt_config, 0, sizeof(mqtt_config));
    mqtt_config.broker.address.uri = AWS_MQTT_BROKER_ENDPOINT_URL;

    if (auth_get_which() == AUTH_PROVISION){
        ESP_LOGI(TAG, "USE PROVISION AUTH");
        mqtt_config.broker.verification.certificate = auth_aws_provision_root_ca;
        mqtt_config.credentials.authentication.certificate = auth_aws_provision_thing_cert;
        mqtt_config.credentials.authentication.key = auth_aws_provision_thing_key;
    }
    if (auth_get_which() == AUTH_OTA){
        ESP_LOGI(TAG, "USE OTA AUTH");
        mqtt_config.broker.verification.certificate = auth_aws_ota_root_ca;
        mqtt_config.credentials.authentication.certificate = auth_aws_ota_thing_cert;
        mqtt_config.credentials.authentication.key = auth_aws_ota_thing_key;
    }

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_config);
    // TODO: This seems a bit shit
    mqtt_client = client;
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_mqtt_client_start(client);

    return true;
}
