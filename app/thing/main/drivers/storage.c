/*
 * nvs.c
 *
 *  Created on: 25 may 2023
 *      Author: klaslofstedt
 */
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <string.h>
#include "drivers/storage.h"

// Values have to be <= 15 characters
#define NVS_NAMESPACE                   "storage"
#define STORAGE_KEY_MAX_LEN             15 // NVS_KEY_NAME_MAX_SIZE -1 // TODO does this exist in esp-idf?

static const char *TAG = "NVS";

static SemaphoreHandle_t storage_lock = NULL;

static bool storage_take_lock(void);
static void storage_give_lock(void);


static bool storage_take_lock(void)
{
    if (!(xSemaphoreTake(storage_lock, 0) == pdTRUE)){
        ESP_LOGE(TAG, "Error: xSemaphoreTake");
        return false;
    }
    return true;
}

static void storage_give_lock(void)
{
    xSemaphoreGive(storage_lock);
}

bool storage_has_flags(const char* key, uint32_t flags)
{
    if (!storage_take_lock()){
        ESP_LOGE(TAG, "Error: storage_take_lock");
        return false;
    }
    if (strlen(key) > STORAGE_KEY_MAX_LEN){
        ESP_LOGE(TAG, "Error: Storage key %s is too long (max 15 characters)", key);
        storage_give_lock();
        return false;
    }
    nvs_handle_t handle;
    esp_err_t err;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Opening NVS failed: %d ", err);
        storage_give_lock();
        return false;
    }
    // Get the current flags value
    uint32_t current_flags = 0;
    err = nvs_get_u32(handle, key, &current_flags);
    nvs_close(handle);

    // If getting the flags value failed for a reason other than the key not being found, propagate the error
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error: Failed to get NVS flags: %d", err);
        storage_give_lock();
        return false;
    }
    // Check if the flag is set
    if ((current_flags & flags) == flags) {
        storage_give_lock();
        return true;
    } else {
        storage_give_lock();
        ESP_LOGE(TAG, "Error: Failed to check NVS flags: %d", err);
        return false;
    }
}

bool storage_set_flags(const char* key, uint32_t flags)
{
    if (!storage_take_lock()){
        ESP_LOGE(TAG, "Error: storage_take_lock");
        return false;
    }
    if (strlen(key) > STORAGE_KEY_MAX_LEN){
        ESP_LOGE(TAG, "Error: Storage key %s is too long (max 15 characters)", key);
        storage_give_lock();
        return false;
    }
    nvs_handle_t handle;
    esp_err_t err;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Opening NVS failed: %d ", err);
        storage_give_lock();
        return false;
    }

    // Get the current flags value
    uint32_t current_flags = 0;
    err = nvs_get_u32(handle, key, &current_flags);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error: Failed to get NVS flags: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    // Set the requested flag
    current_flags |= flags;

    // Write flags back to NVS
    err = nvs_set_u32(handle, key, current_flags);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Failed to set NVS flag: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    // Commit the write operation
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: committing NVS failed: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    // Close NVS
    nvs_close(handle);
    storage_give_lock();
    return true;
}

bool storage_unset_flags(const char* key, uint32_t flags)
{
    if (!storage_take_lock()){
        ESP_LOGE(TAG, "Error: storage_take_lock");
        return false;
    }
    if (strlen(key) > STORAGE_KEY_MAX_LEN){
        ESP_LOGE(TAG, "Error: Storage key %s is too long (max 15 characters)", key);
        storage_give_lock();
        return false;
    }
    nvs_handle_t handle;
    esp_err_t err;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Opening NVS failed: %d ", err);
        storage_give_lock();
        return false;
    }

    // Get the current flags value
    uint32_t current_flags = 0;
    err = nvs_get_u32(handle, key, &current_flags);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error: Failed to get NVS flags: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    // Unset the requested flags
    current_flags &= ~flags;
    // Write flags back to NVS
    err = nvs_set_u32(handle, key, current_flags);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Failed to unset NVS flags: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    // Commit the write operation
    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: committing NVS failed: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    // Close NVS
    nvs_close(handle);
    storage_give_lock();
    return true;
}

bool storage_erase_blob(const char* key) 
{
    if (!storage_take_lock()){
        ESP_LOGE(TAG, "Error: storage_take_lock");
        return false;
    }
    if (strlen(key) > STORAGE_KEY_MAX_LEN){
        ESP_LOGE(TAG, "Error: Storage key %s is too long (max 15 characters)", key);
        storage_give_lock();
        return false;
    }
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Opening NVS failed: %d ", err);
        storage_give_lock();
        return false;
    }

    err = nvs_erase_key(handle, key);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Error: Key not found");
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: erasing key failed: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: committing NVS failed: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    nvs_close(handle);
    storage_give_lock();
    return true;
}

bool storage_append_blob(const char* key, const char* buffer, size_t max_size)
{
    if (!storage_take_lock()){
        ESP_LOGE(TAG, "Error: storage_take_lock");
        return false;
    }
    if (strlen(key) > STORAGE_KEY_MAX_LEN){
        ESP_LOGE(TAG, "Error: Storage key %s is too long (max 15 characters)", key);
        storage_give_lock();
        return false;
    }
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Opening NVS failed: %d ", err);
        storage_give_lock();
        return false;
    }

    size_t old_size;
    err = nvs_get_blob(handle, key, NULL, &old_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        old_size = 0;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: buffer size failed: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    size_t buffer_size = strlen(buffer) + 1;
    size_t new_size = old_size + buffer_size;

    if (new_size > max_size){
        ESP_LOGE(TAG, "Error: New size too large");
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    char* new_data = (char*) malloc(new_size);
    if (new_data == NULL) {
        ESP_LOGE(TAG, "Error: allocating memory failed");
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    if (old_size > 0) {
        err = nvs_get_blob(handle, key, new_data, &old_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error: reading existing blob failed: %d", err);
            free(new_data);
            nvs_close(handle);
            storage_give_lock();
            return false;
        }
    }

    strncpy(new_data + old_size, buffer, buffer_size - 1);
    new_data[old_size + buffer_size - 1] = '\n';

    err = nvs_set_blob(handle, key, new_data, new_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: setting blob failed: %d", err);
    } else {
        err = nvs_commit(handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error: committing NVS failed: %d", err);
        }
    }

    free(new_data);
    nvs_close(handle);
    storage_give_lock();
    return true;
}

bool storage_set_blob(const char* key, const char* buffer, size_t size)
{
    if (!storage_take_lock()){
        ESP_LOGE(TAG, "Error: storage_take_lock");
        return false;
    }
    if (strlen(key) > STORAGE_KEY_MAX_LEN){
        ESP_LOGE(TAG, "Error: Storage key %s is too long (max 15 characters)", key);
        storage_give_lock();
        return false;
    }
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Opening NVS failed: %d ", err);
        storage_give_lock();
        return false;
    }

    // Directly set the blob without any previous data considerations
    err = nvs_set_blob(handle, key, buffer, size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: setting blob failed: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: committing NVS failed: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    nvs_close(handle);
    storage_give_lock();
    return true;
}

bool storage_get_blob(const char* key, char* blob, size_t size) 
{
    if (!storage_take_lock()){
        ESP_LOGE(TAG, "Error: storage_take_lock");
        return false;
    }
    if (strlen(key) > STORAGE_KEY_MAX_LEN){
        ESP_LOGE(TAG, "Error: Storage key %s is too long (max 15 characters)", key);
        storage_give_lock();
        return false;
    }
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Opening NVS failed: %d", err);
        storage_give_lock();
        return false;
    }

    size_t blob_size;
    err = nvs_get_blob(handle, key, NULL, &blob_size);
    if (err != ESP_OK || blob_size > size) {
        ESP_LOGE(TAG, "Error: Blob size too large: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    err = nvs_get_blob(handle, key, blob, &blob_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Getting blob failed: %d", err);
        nvs_close(handle);
        storage_give_lock();
        return false;
    }

    nvs_close(handle);
    storage_give_lock();
    return true;
}

bool storage_init(void)
{
    ESP_LOGI(TAG, "Initialise");
    storage_lock = xSemaphoreCreateBinary();
    storage_give_lock();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error: Storage has no free pages");
            return false;
        }
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Storage failed to initialise");
        return false;
    }
    return true;
}
