/*
 * ota.c
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_encrypted_img.h"
#include "drivers/storage.h"
#include "utilities/auth_aws_ota.h"
#include "utilities/auth_aws_provision.h"
#include "app/ota.h"
#include "middlewares/mqtt.h"
#include <cJSON.h>
#include "middlewares/auth.h"


#define OTA_URL_MAX_SIZE        MQTT_DATA_MAX_LEN
#define OTA_STORAGE_KEY_URL     "ota_url"
#define OTA_STORAGE_KEY_FLAGS   "ota_flags"

static char ota_url_buffer[OTA_URL_MAX_SIZE];

static const char *TAG = "OTA";
extern const char rsa_private_pem_start[] asm("_binary_rsa_key_pem_start");
extern const char rsa_private_pem_end[]   asm("_binary_rsa_key_pem_end");

static esp_err_t ota_validate_header(esp_app_desc_t *new_app_info);
static esp_err_t ota_decrypt_cb(decrypt_cb_arg_t *args, void *user_ctx);

static esp_err_t ota_validate_header(esp_app_desc_t *new_app_info)
{
    if (new_app_info == NULL) {
        ESP_LOGE(TAG, "Error: ota_validate_header invalid argument");
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

    return ESP_OK;
}

static esp_err_t ota_decrypt_cb(decrypt_cb_arg_t *args, void *user_ctx)
{
    if (args == NULL || user_ctx == NULL) {
        ESP_LOGE(TAG, "Error: ota_decrypt_cb Invalid argument");
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t err;
    pre_enc_decrypt_arg_t pargs = {};
    pargs.data_in = args->data_in;
    pargs.data_in_len = args->data_in_len;
    err = esp_encrypted_img_decrypt_data((esp_decrypt_handle_t *)user_ctx, &pargs);
    if (err != ESP_OK && err != ESP_ERR_NOT_FINISHED) {
        ESP_LOGE(TAG, "Error: esp_encrypted_img_decrypt_data");
        return err;
    }
    static bool is_image_verified = false;
    if (pargs.data_out_len > 0) {
        args->data_out = pargs.data_out;
        args->data_out_len = pargs.data_out_len;
        if (!is_image_verified) {
            is_image_verified = true;
            const int app_desc_offset = sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t);
            // It is unlikely to not have App Descriptor available in first iteration of decrypt callback.
            if (args->data_out_len < app_desc_offset + sizeof(esp_app_desc_t)) {
                ESP_LOGE(TAG, "Error: ota_decrypt_cb data_out_len is less than required");
                free(pargs.data_out);
                return ESP_ERR_INVALID_SIZE;
            }
            esp_app_desc_t *app_info = (esp_app_desc_t *) &args->data_out[app_desc_offset];
            err = ota_validate_header(app_info);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error: ota_validate_header");
                free(pargs.data_out);
            }
            ESP_LOGE(TAG, "Error: is_image_verified failed");
            return err;
        }
    } else {
        args->data_out_len = 0;
    }

    return ESP_OK;
}

bool ota_set_url(const char* json_str)
{
    cJSON *root = NULL;
    root = cJSON_Parse(json_str);
    if(root == NULL){
        ESP_LOGE(TAG, "Error: root failed to parse");
        cJSON_Delete(root);
        return false;
    }

    cJSON *do_ota = cJSON_GetObjectItemCaseSensitive(root, "do_ota");
    if (!cJSON_IsBool(do_ota)) {
        ESP_LOGE(TAG, "Error: do_ota is not a bool");
        cJSON_Delete(root);
        return false;
    }

    if (!cJSON_IsTrue(do_ota)){
        ESP_LOGI(TAG, "Error: do_ota is false");
        cJSON_Delete(root);
        return false;
    }

    cJSON *otaurl = cJSON_GetObjectItemCaseSensitive(root, "otaurl");
    if (!cJSON_IsString(otaurl)) {
        ESP_LOGI(TAG, "Error: otaurl is not a string");
        cJSON_Delete(root);
        return false;
    }
    size_t url_len = strlen(otaurl->valuestring) + 1;
    if (url_len > OTA_URL_MAX_SIZE) {
        ESP_LOGE(TAG, "Error: URL is too long");
        cJSON_Delete(root);
        return false;
    }
    strncpy(ota_url_buffer, otaurl->valuestring, OTA_URL_MAX_SIZE);

    // Ensure the buffer is null-terminated.
    ota_url_buffer[OTA_URL_MAX_SIZE - 1] = '\0';

    cJSON_Delete(root);
    
    return true;
}

bool ota_run(void)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    esp_err_t ota_finish_err = ESP_OK;
    esp_http_client_config_t client_config;
    memset(&client_config, 0, sizeof(client_config));
    client_config.url = ota_url_buffer;
    client_config.timeout_ms = CONFIG_EXAMPLE_OTA_RECV_TIMEOUT;
    client_config.keep_alive_enable = true;
    client_config.buffer_size_tx = OTA_URL_MAX_SIZE;
    ESP_LOGI(TAG, "OTA URL: %s", ota_url_buffer);

    if (auth_get_which() == AUTH_PROVISION){
        ESP_LOGI(TAG, "USE PROVISION AUTH");
        client_config.cert_pem = auth_aws_provision_root_ca;
    }
    if (auth_get_which() == AUTH_OTA){
        ESP_LOGI(TAG, "USE OTA AUTH");
        client_config.cert_pem = auth_aws_ota_root_ca;
    }
    esp_decrypt_cfg_t cfg = {};
    cfg.rsa_pub_key = rsa_private_pem_start;
    cfg.rsa_pub_key_len = rsa_private_pem_end - rsa_private_pem_start;
    esp_decrypt_handle_t decrypt_handle = esp_encrypted_img_decrypt_start(&cfg);
    if (!decrypt_handle) {
        ESP_LOGE(TAG, "Error: OTA upgrade failed");
        return false;
    }

    esp_https_ota_config_t ota_config = {
        .http_config = &client_config,
        .decrypt_cb = ota_decrypt_cb,
        .decrypt_user_ctx = (void *)decrypt_handle,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_https_ota_begin failed");
        return false;
    }

    while (true) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        // esp_https_ota_perform returns after every read operation which gives user the ability to
        // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
        // data read so far.
        ESP_LOGI(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
    }

    if (!esp_https_ota_is_complete_data_received(https_ota_handle)) {
        // the OTA image was not completely received and user can customise the response to this situation.
        ESP_LOGE(TAG, "Complete data was not received.");
    } else {
        err = esp_encrypted_img_decrypt_end(decrypt_handle);
        if (err != ESP_OK) {
            esp_https_ota_abort(https_ota_handle);
            ESP_LOGE(TAG, "Error: esp_encrypted_img_decrypt_end failed");
            return false;
        }
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
            auth_use_ota();
            ESP_LOGI(TAG, "OTA upgrade successful");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            // OTA successful, return false to reboot
            return false;
        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                ESP_LOGE(TAG, "Error: Image validation failed, image is corrupted");
            }
            ESP_LOGE(TAG, "Error: ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
            return false;
        }
    }

    return false;
}
