#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include "app/thing.h"
#include "esp_mac.h"
#include <esp_wifi.h>
#include "utilities/misc.h"
#include <cJSON.h>


static const char *TAG = "MISC";

#define HEX_TO_INT(c) ((c) >= '0' && (c) <= '9' ? (c) - '0' : ((c) >= 'a' && (c) <= 'f' ? (c) - 'a' + 10 : ((c) >= 'A' && (c) <= 'F' ? (c) - 'A' + 10 : -1)))

bool hex_string_to_u8_array(char *in, uint8_t *out, size_t size_out)
{
    for (size_t i = 0; i < size_out; ++i) {
        int high_nibble = HEX_TO_INT(in[2 * i]);
        int low_nibble = HEX_TO_INT(in[2 * i + 1]);
        if (high_nibble < 0 || low_nibble < 0) {
            ESP_LOGE(TAG, "Error: Failed to convert string to u8 array");
            return false;
        }
        out[i] = (uint8_t)((high_nibble << 4) | low_nibble);
    }
    return true;
}

void u8_array_to_ascii_string(const unsigned char *bytes, char *ascii_string, size_t size_in)
{
    for (size_t i = 0; i < size_in; i++) {
        ascii_string[i] = (char)bytes[i];
    }
    ascii_string[size_in] = '\0';
}

bool id_get(char *thing_id)
{
    uint8_t mac[6];
    const char *prefix = "id";
    esp_efuse_mac_get_default(mac);
    snprintf(thing_id, ID_SIZE, "%s%02X%02X%02X%02X%02X%02X",
             prefix, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (strncmp(thing_id, "id000000000000", ID_SIZE) == 0){
        ESP_LOGE(TAG, "Error: ID invalid == id000000000000");
        return false;
    }
    return true;
}


