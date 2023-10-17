/*
 * mobile.c
 *
 *  Created on: 21 jul 2023
 *      Author: klaslofstedt
 */
#include <stdbool.h>
#include <cJSON.h>
#include "esp_log.h"
#include "app/mobile.h"
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


static const char *TAG = "MOBILE";

static SemaphoreHandle_t mobile_value_lock = NULL;


static mobile_value_t mobile_value_g = {
    .read = {
        .nickname = "",
        .sw_version = ""
    },
    .readwrite = {
        .network = "online",
    }
};

static bool mobile_get_struct(mobile_value_t *mobile_value);
static bool mobile_set_struct(mobile_value_t mobile_value);
static bool mobile_take_lock(void);
static void mobile_give_lock(void);


static bool mobile_take_lock(void)
{
    if (!(xSemaphoreTake(mobile_value_lock, 0) == pdTRUE)){
        ESP_LOGE(TAG, "Error: xSemaphoreTake");
        return false;
    }
    return true;
}

static void mobile_give_lock(void)
{
    xSemaphoreGive(mobile_value_lock);
}

static bool mobile_get_struct(mobile_value_t *mobile_value) 
{    
    if(!mobile_take_lock()) {
        ESP_LOGE(TAG, "Error: mobile_take_lock");
        return false;
    }
    *mobile_value = mobile_value_g; 
    mobile_give_lock();
    return true;
}

static bool mobile_set_struct(mobile_value_t mobile_value) 
{
    if(!mobile_take_lock()) {
        ESP_LOGE(TAG, "Error: mobile_take_lock");
        return false;
    }
    mobile_value_g = mobile_value;
    mobile_give_lock();
    return true;
}

bool mobile_set_value_json(const char* json_str)
{
    cJSON *root = NULL;
    root = cJSON_Parse(json_str);
    if(root == NULL){
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: root == NULL");
        return false;
    }

    // Check that mobile_value exists
    cJSON *mobile_value = cJSON_GetObjectItemCaseSensitive(root, "mobile_value");
    if (!cJSON_IsObject(mobile_value)) {
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: cJSON_IsObject mobile_value");
        return false;
    }

    cJSON *mobile_value_readwrite = cJSON_GetObjectItemCaseSensitive(mobile_value, "readwrite");
    if (!cJSON_IsObject(mobile_value_readwrite)) {
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: cJSON_IsObject mobile_value_readwrite");
        return false;
    }

    cJSON *network = cJSON_GetObjectItemCaseSensitive(mobile_value_readwrite, "network");
    if (!cJSON_IsString(network)) {
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: cJSON_IsString network");
        return false;
    }

    cJSON *mobile_value_read = cJSON_GetObjectItemCaseSensitive(mobile_value, "read");
    if (!cJSON_IsObject(mobile_value_read)) {
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: cJSON_IsObject mobile_value_read");
        return false;
    }

    cJSON *nickname = cJSON_GetObjectItemCaseSensitive(mobile_value_read, "nickname");
    if (!cJSON_IsString(nickname)) {
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: cJSON_IsString nickname");
        return false;
    }

    cJSON *sw_version = cJSON_GetObjectItemCaseSensitive(mobile_value_read, "sw_version");
    if (!cJSON_IsString(sw_version)) {
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: cJSON_IsString sw_version");
        return false;
    }

    // Update the global thing struct thread safely
    mobile_value_t mobile;
    if (!mobile_get_struct(&mobile)) {
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: mobile_get_struct");
        return false;
    }

    strncpy(mobile.readwrite.network, network->valuestring, MOBILE_STRING_MAX_LEN);
    mobile.readwrite.network[MOBILE_STRING_MAX_LEN - 1] = '\0';

    strncpy(mobile.read.nickname, nickname->valuestring, MOBILE_STRING_MAX_LEN);
    mobile.read.nickname[MOBILE_STRING_MAX_LEN - 1] = '\0';

    strncpy(mobile.read.sw_version, sw_version->valuestring, MOBILE_STRING_MAX_LEN);
    mobile.read.sw_version[MOBILE_STRING_MAX_LEN - 1] = '\0';

    if (!mobile_set_struct(mobile)) {
        cJSON_Delete(root);
        ESP_LOGE(TAG, "Error: mobile_set_struct");
        return false;
    }
    cJSON_Delete(root);

    return true;
}

bool mobile_get_value_json(cJSON* root)
{
    mobile_value_t mobile;
    if (!mobile_get_struct(&mobile)){
        ESP_LOGE(TAG, "Error: mobile_get_value_json");
        return false;
    }
    
    cJSON* readwrite = cJSON_CreateObject();
    cJSON* read = cJSON_CreateObject();
    cJSON_AddStringToObject(readwrite, "network", "online");
    cJSON_AddStringToObject(read, "nickname", mobile.read.nickname);
    cJSON_AddStringToObject(read, "sw_version", mobile.read.sw_version);
    cJSON_AddItemToObject(root, "readwrite", readwrite);
    cJSON_AddItemToObject(root, "read", read);

    return true;
}

bool mobile_init(void)
{
    ESP_LOGI(TAG, "Initialise");
    mobile_value_lock = xSemaphoreCreateBinary();
    mobile_give_lock();
    return true;
}