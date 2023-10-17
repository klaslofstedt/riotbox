/*
 * aws.c
 *
 *  Created on: 17 sep 2023
 *      Author: klaslofstedt
 */
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include "utilities/auth_aws_provision.h"
#include "drivers/storage.h"
#include "esp_log.h"

#define AUTH_AWS_STORAGE_KEY_FLAGS                  "auth_aws_flags"
#define AUTH_AWS_PROVISION_STORAGE_KEY_ROOT_CA      "aws_root_ca"
#define AUTH_AWS_PROVISION_STORAGE_KEY_THING_CERT   "aws_thing_cert"
#define AUTH_AWS_PROVISION_STORAGE_KEY_THING_KEY    "aws_thing_key"

static const char *TAG = "AUTH_AWS_PROV";


char auth_aws_provision_root_ca[AUTH_AWS_PROVISION_ROOT_CA_BUFFER_SIZE];
char auth_aws_provision_thing_cert[AUTH_AWS_PROVISION_THING_CERT_BUFFER_SIZE];
char auth_aws_provision_thing_key[AUTH_AWS_PROVISION_THING_KEY_BUFFER_SIZE];

bool auth_aws_provision_set_has_root_ca(void)
{
    return storage_set_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_ROOT_CA);
}

bool auth_aws_provision_unset_has_root_ca(void)
{
    return storage_unset_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_ROOT_CA);
}

bool auth_aws_provision_set_has_thing_cert(void)
{
    return storage_set_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_THING_CERT);
}

bool auth_aws_provision_unset_has_thing_cert(void)
{
    return storage_unset_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_THING_CERT);
}

bool auth_aws_provision_set_has_thing_key(void)
{
    return storage_set_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_THING_KEY);
}

bool auth_aws_provision_unset_has_thing_key(void)
{
    return storage_unset_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_THING_KEY);
}

bool auth_aws_provision_get_has_root_ca(void)
{
    return storage_has_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_ROOT_CA);
}

bool auth_aws_provision_get_has_thing_cert(void)
{
    return storage_has_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_THING_CERT);
}

bool auth_aws_provision_get_has_thing_key(void)
{
    return storage_has_flags(AUTH_AWS_STORAGE_KEY_FLAGS, AUTH_AWS_PROVISION_HAS_THING_KEY);
}

bool auth_aws_provision_load_root_ca(void)
{
    if (!auth_aws_provision_get_has_root_ca()){
        ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_root_ca");
        return false;
    }
    return storage_get_blob(AUTH_AWS_PROVISION_STORAGE_KEY_ROOT_CA, auth_aws_provision_root_ca, AUTH_AWS_PROVISION_ROOT_CA_BUFFER_SIZE);
}

bool auth_aws_provision_append_root_ca(const char* blob)
{
    if (auth_aws_provision_get_has_root_ca()){
        ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_root_ca");
        return false;
    }
    if (!storage_append_blob(AUTH_AWS_PROVISION_STORAGE_KEY_ROOT_CA, blob, AUTH_AWS_PROVISION_ROOT_CA_BUFFER_SIZE)){
        ESP_LOGE(TAG, "Error: storage_append_blob");
        return false;
    }
    return true;
}

bool auth_aws_provision_erase_root_ca(void) 
{
    if (!storage_erase_blob(AUTH_AWS_PROVISION_STORAGE_KEY_ROOT_CA)){
        ESP_LOGE(TAG, "Error: storage_erase_blob");
        return false;
    }
    return auth_aws_provision_unset_has_root_ca();
}

bool auth_aws_provision_load_thing_cert(void)
{
    if (!auth_aws_provision_get_has_thing_cert()){
        ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_thing_cert");
        return false;
    }
    return storage_get_blob(AUTH_AWS_PROVISION_STORAGE_KEY_THING_CERT, auth_aws_provision_thing_cert, AUTH_AWS_PROVISION_THING_CERT_BUFFER_SIZE);
}

bool auth_aws_provision_append_thing_cert(const char* blob)
{
    if (auth_aws_provision_get_has_thing_cert()){
        ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_thing_cert");
        return false;
    }
    if (!storage_append_blob(AUTH_AWS_PROVISION_STORAGE_KEY_THING_CERT, blob, AUTH_AWS_PROVISION_THING_CERT_BUFFER_SIZE)){
        ESP_LOGE(TAG, "Error: storage_append_blob");
        return false;
    }
    return true;
}

bool auth_aws_provision_erase_thing_cert(void) 
{
    if (!storage_erase_blob(AUTH_AWS_PROVISION_STORAGE_KEY_THING_CERT)){
        ESP_LOGE(TAG, "Error: storage_erase_blob");
        return false;
    }
    return auth_aws_provision_unset_has_thing_cert();
}

bool auth_aws_provision_load_thing_key(void)
{
    if (!auth_aws_provision_get_has_thing_key()){
        ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_thing_key");
        return false;
    }
    return storage_get_blob(AUTH_AWS_PROVISION_STORAGE_KEY_THING_KEY, auth_aws_provision_thing_key, AUTH_AWS_PROVISION_THING_KEY_BUFFER_SIZE);
}

bool auth_aws_provision_append_thing_key(const char* blob)
{
    if (auth_aws_provision_get_has_thing_key()){
        ESP_LOGE(TAG, "Error: auth_aws_provision_get_has_thing_key");
        return false;
    }
    if (!storage_append_blob(AUTH_AWS_PROVISION_STORAGE_KEY_THING_KEY, blob, AUTH_AWS_PROVISION_THING_KEY_BUFFER_SIZE)){
        ESP_LOGE(TAG, "Error: storage_append_blob");
        return false;
    }
    return true;
}

bool auth_aws_provision_erase_thing_key(void) 
{
    if (!storage_erase_blob(AUTH_AWS_PROVISION_STORAGE_KEY_THING_KEY)){
        ESP_LOGE(TAG, "Error: storage_erase_blob");
        return false;
    }
    return auth_aws_provision_unset_has_thing_key();
}