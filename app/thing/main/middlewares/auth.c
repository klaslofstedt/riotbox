/*
 * auth.c
 *
 *  Created on: 17 sep 2023
 *      Author: klaslofstedt
 */
#include "middlewares/auth.h"
#include "utilities/auth_aws_provision.h"
#include "utilities/auth_aws_ota.h"
#include "drivers/storage.h"
#include "esp_log.h"
#include <string.h>

#define AUTH_STORAGE_KEY_FLAGS         "auth_flags"

static const char *TAG = "AUTH";

static auth_t auth_use = AUTH_OTA;

bool auth_init(void)
{
    ESP_LOGI(TAG, "Initialise");

    if (storage_has_flags(AUTH_STORAGE_KEY_FLAGS, AUTH_USE_OTA)){
        auth_use = AUTH_OTA;
    }
    if (storage_has_flags(AUTH_STORAGE_KEY_FLAGS, AUTH_USE_PROVISION)){
        if (auth_aws_provision_get_has_root_ca()){
            if (!auth_aws_provision_load_root_ca()){
                auth_use = AUTH_OTA;
                ESP_LOGE(TAG, "Error: auth_aws_provision_load_root_ca. Attempting to use AUTH_OTA");
                return false;
            }
        } else {
            auth_use = AUTH_OTA;
            ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_root_ca. Attempting to use AUTH_OTA");
            return false;
        }
        if (auth_aws_provision_get_has_thing_cert()){
            if (!auth_aws_provision_load_thing_cert()){
                auth_use = AUTH_OTA;
                ESP_LOGE(TAG, "Error: auth_aws_provision_load_thing_cert. Attempting to use AUTH_OTA");
                return false;
            }
        } else {
            auth_use = AUTH_OTA;
            ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_thing_cert. Attempting to use AUTH_OTA");
            return false;
        }
        if (auth_aws_provision_get_has_thing_key()){
            if (!auth_aws_provision_load_thing_key()){
                auth_use = AUTH_OTA;
                ESP_LOGE(TAG, "Error: auth_aws_provision_load_thing_key. Attempting to use AUTH_OTA");
                return false;
            }
        } else {
            auth_use = AUTH_OTA;
            ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_thing_key. Attempting to use AUTH_OTA");
            return false;
        }
        auth_use = AUTH_PROVISION;
    }
    return true;
}

bool auth_use_ota(void)
{
    if (!storage_set_flags(AUTH_STORAGE_KEY_FLAGS, AUTH_USE_OTA)){
        ESP_LOGE(TAG, "Error: storage_set_flags");
        return false;
    }
    if (!storage_unset_flags(AUTH_STORAGE_KEY_FLAGS, AUTH_USE_PROVISION)){
        ESP_LOGE(TAG, "Error: storage_unset_flags");
        return false;
    }
    return true;
}

bool auth_use_provision(void)
{
    if (!storage_set_flags(AUTH_STORAGE_KEY_FLAGS, AUTH_USE_PROVISION)){
        ESP_LOGE(TAG, "Error: storage_set_flags");
        return false;
    }
    if (!storage_unset_flags(AUTH_STORAGE_KEY_FLAGS, AUTH_USE_OTA)){
        ESP_LOGE(TAG, "Error: storage_unset_flags");
        return false;
    }
    return true;
}

auth_t auth_get_which(void)
{
    return auth_use;
}