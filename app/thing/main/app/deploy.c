/*
 * deploy.c
 *
 *  Created on: 25 jun 2023
 *      Author: klaslofstedt
 */
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <inttypes.h>
#include <stdio.h>
#include "esp_log.h"
#include "app/deploy.h"
#include "middlewares/ble.h"
#include "utilities/aes.h"
#include "app/thing.h"
#include "mbedtls/base64.h"
#include <cJSON.h>
#include <string.h>
#include "utilities/misc.h"
#include "board/board.h"

#define BASE64_ENCODE_OUT_SIZE(size)    ((((4 * size) + 2) / 3) + 4)
#define BASE64_DECODE_OUT_SIZE(size)    ((size / 4) * 3)

static const char *TAG = "DEPLOY";

static uint8_t base64_encode_buffer[BASE64_ENCODE_OUT_SIZE(DEPLOY_UART_BUFFER_SIZE)];
static uint8_t base64_decode_buffer[BASE64_DECODE_OUT_SIZE(DEPLOY_UART_BUFFER_SIZE)];

static uint8_t uart_buffer[DEPLOY_UART_BUFFER_SIZE];

static bool deploy_uart_init(void);
static bool deploy_write_thing_id(void);
static bool deploy_read_thing_blob(void);
static bool deploy_write_status(bool status);
static bool deploy(void);

bool deploy_is_valid(void)
{
    if (!ble_get_has_pop()){
        return false;
    }
    if (!aes_get_has_key()){
        return false;
    }
    if (!thing_get_has_type()){
        return false;
    }
    if (!thing_get_has_hw_version()){
        return false;
    }
    return true;
}

static bool deploy_write_thing_id(void)
{
    char thing_id[ID_SIZE];
    if (!id_get(thing_id)){
        return false;
    }
    cJSON *message_root = cJSON_CreateObject();
    cJSON_AddStringToObject(message_root, "id", thing_id);
    char *json_str = cJSON_Print(message_root);
    size_t base64_size;
    mbedtls_base64_encode(NULL, 0, &base64_size, (unsigned char*)json_str, strlen(json_str));
    mbedtls_base64_encode(base64_encode_buffer, sizeof(base64_encode_buffer), &base64_size, (unsigned char*)json_str, strlen(json_str));

    if (!uart_write_bytes(DEPLOY_UART_PORT_NUM, base64_encode_buffer, base64_size)){
        cJSON_Delete(message_root);
        free(json_str);
        return false;
    }
    cJSON_Delete(message_root);
    free(json_str);

    return true;
}

static bool deploy_read_thing_blob(void)
{
    size_t bytes_read = uart_read_bytes(DEPLOY_UART_PORT_NUM, uart_buffer, DEPLOY_UART_BUFFER_SIZE, 1000 / portTICK_PERIOD_MS);
    if (!bytes_read){
        return false;
    }

    size_t size;
    if (mbedtls_base64_decode(base64_decode_buffer, sizeof(base64_decode_buffer), &size, uart_buffer, bytes_read) != 0){
        return false;
    }

    cJSON *root = cJSON_Parse((char *)base64_decode_buffer);
    cJSON *pop = cJSON_GetObjectItemCaseSensitive(root, "pop");
    if (cJSON_IsString(pop) && (pop->valuestring != NULL)) {
    } else {
        cJSON_Delete(root);
        return false;
    }
    cJSON *aes = cJSON_GetObjectItemCaseSensitive(root, "aes");
    if (cJSON_IsString(aes) && (aes->valuestring != NULL)) {
    } else {
        cJSON_Delete(root);
        return false;
    }
    cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");
    if (cJSON_IsString(type) && (type->valuestring != NULL)) {
    } else {
        cJSON_Delete(root);
        return false;
    }
    cJSON *hw_version = cJSON_GetObjectItemCaseSensitive(root, "hw_version");
    if (cJSON_IsString(hw_version) && (hw_version->valuestring != NULL)) {
    } else {
        cJSON_Delete(root);
        return false;
    }

    if (!thing_set_type(type->valuestring)){
        cJSON_Delete(root);
        return false;
    }

    if (!thing_set_hw_version(hw_version->valuestring)){
        cJSON_Delete(root);
        return false;
    }
    
    if (!ble_set_pop(pop->valuestring)){
        cJSON_Delete(root);
        return false;
    }

    if (!aes_set_key(aes->valuestring)){
        cJSON_Delete(root);
        return false;
    }

    cJSON_Delete(root);
    return true;
}

static bool deploy_write_status(bool status)
{
    cJSON *message_root = cJSON_CreateObject();
    if (status){
        cJSON_AddStringToObject(message_root, "status", "ok");
    } else {
        cJSON_AddStringToObject(message_root, "status", "error");
    }
    char *json_str = cJSON_Print(message_root);
    size_t base64_size;
    mbedtls_base64_encode(NULL, 0, &base64_size, (unsigned char*)json_str, strlen(json_str));
    mbedtls_base64_encode(base64_encode_buffer, sizeof(base64_encode_buffer), &base64_size, (unsigned char*)json_str, strlen(json_str));
    int bytes_sent = uart_write_bytes(DEPLOY_UART_PORT_NUM, base64_encode_buffer, base64_size);
    cJSON_Delete(message_root);
    free(json_str);

    if (!bytes_sent){
        return false;
    }
    return true;
}

bool deploy(void)
{
    while(true){
        bool do_next = true;
        if (do_next){
            do_next = deploy_write_thing_id();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        if (do_next){
            do_next = deploy_read_thing_blob();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        if (do_next){
            do_next = deploy_write_status(true);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        if (do_next){
            return true;
        }
    }
    return false;
}

static bool deploy_uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = DEPLOY_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

    if (uart_driver_install(DEPLOY_UART_PORT_NUM, DEPLOY_UART_BUFFER_SIZE * 2, 0, 0, NULL, intr_alloc_flags) != ESP_OK){
        return false;
    }
    if (uart_param_config(DEPLOY_UART_PORT_NUM, &uart_config) != ESP_OK) {
        return false;
    }
    if (uart_set_pin(DEPLOY_UART_PORT_NUM, DEPLOY_UART_TXD, DEPLOY_UART_RXD, DEPLOY_UART_RTS, DEPLOY_UART_CTS) != ESP_OK){
        return false;
    }
    return true;
}

bool deploy_init(void)
{
    ESP_LOGI(TAG, "Initialise");
    if (deploy_is_valid()){
        esp_log_level_set("*", ESP_LOG_INFO);
        return true;
    }
    esp_log_level_set("*", ESP_LOG_NONE);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    if (!deploy_uart_init()){
        ESP_LOGI(TAG, "Failed to initialise Deploy Uart");
        return false;
    }
    
    return deploy();
}