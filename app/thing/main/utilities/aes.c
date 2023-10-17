#include "mbedtls/aes.h"
#include "esp_log.h"
#include <string.h>
#include "utilities/aes.h"
#include "drivers/storage.h"
#include "utilities/misc.h"
#include "esp_log.h"

#define AES_STORAGE_KEY_FLAGS       "aes_flags"
#define AES_STORAGE_KEY_AES_KEY     "aes_key"

static const char *TAG = "AES";

static mbedtls_aes_context aes;

static bool aes_set_has_key(void);

static bool aes_set_has_key(void)
{
    return storage_set_flags(AES_STORAGE_KEY_FLAGS, AES_HAS_KEY);
}

bool aes_get_has_key(void)
{
    return storage_has_flags(AES_STORAGE_KEY_FLAGS, AES_HAS_KEY);
}

bool aes_set_key(const char *aes)
{
    if (aes_get_has_key()){
        ESP_LOGE(TAG, "Error: aes_get_has_key");
        return false;
    }
    if (!storage_set_blob(AES_STORAGE_KEY_AES_KEY, aes, 2 * AES_BLOCK_SIZE + 1)){
        ESP_LOGE(TAG, "Error: storage_set_array");
        return false;
    }
    return aes_set_has_key();
}

bool aes_get_key(char *aes)
{
    if (!aes_get_has_key()){
        ESP_LOGE(TAG, "Error: aes_get_has_key");
        return false;
    }
    return storage_get_blob(AES_STORAGE_KEY_AES_KEY, aes, 2 * AES_BLOCK_SIZE + 1);
}

bool aes_init(void)
{
    ESP_LOGI(TAG, "Initialise");
    char aes_key_string[2 * AES_BLOCK_SIZE + 1];
    if (!aes_get_key(aes_key_string)){
        ESP_LOGE(TAG, "Error: aes_get_key");
        return false;
    }
    unsigned char aes_key[AES_BLOCK_SIZE];
    if (!hex_string_to_u8_array(aes_key_string, aes_key, AES_BLOCK_SIZE)){
        ESP_LOGE(TAG, "Error: hex_string_to_u8_array");
        return false;
    }
    mbedtls_aes_init(&aes);
    if (mbedtls_aes_setkey_enc(&aes, (unsigned char *)aes_key, AES_BLOCK_SIZE * 8) != 0){
        ESP_LOGE(TAG, "Error: mbedtls_aes_setkey_enc");
        return false;
    }
    return true;
}

bool aes_deinit()
{
    mbedtls_aes_free(&aes);
    return true;
}

bool aes_crypto(char *received_data, char *decrypted_string, size_t received_data_size)
{
    unsigned char nonce_counter[AES_BLOCK_SIZE];
    memcpy(nonce_counter, received_data, AES_BLOCK_SIZE); // Copy IV bytes directly from the received_data
    
    size_t size = received_data_size - AES_BLOCK_SIZE;
    unsigned char *encrypted_data = (unsigned char *)(received_data + AES_BLOCK_SIZE); // Point directly to the encrypted part

    unsigned char decrypted_u8[size];
    size_t nc_off = 0;
    unsigned char stream_block[AES_BLOCK_SIZE] = {0};
    if (mbedtls_aes_crypt_ctr(&aes, size, &nc_off, nonce_counter, stream_block, encrypted_data, decrypted_u8)){
        ESP_LOGE(TAG, "Error: mbedtls_aes_crypt_ctr");
        return false;
    }
    
    u8_array_to_ascii_string(decrypted_u8, decrypted_string, size);
    return true;
}