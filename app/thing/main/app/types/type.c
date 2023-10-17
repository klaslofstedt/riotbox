// Include your thing type
#include "app/types/type.h"
#include "app/types/switch.h"
#include "app/types/default.h"
#include <string.h>
#include <cJSON.h>
#include "esp_log.h"

static const char *TAG = "TYPE";
static int type_int = 0;

// Add your new type in the if case
bool type_set_int(const char* type_str)
{
    if (strcmp(type_str, SWITCH_TYPE_STR) == 0){
        type_int = SWITCH_TYPE_INT;
        return true;
    }
    ESP_LOGE(TAG, "Error: type_set_int, use default type");
    return false;
}

// Add your new type in the switch case
bool type_init(callback_t callback_publish_value)
{   
    ESP_LOGI(TAG, "Initialise");
    switch(type_int){
        case SWITCH_TYPE_INT: return switch_init(callback_publish_value);
        default: return default_init(callback_publish_value);
    }
    ESP_LOGE(TAG, "Error: type_init");
    return false;
}

// Add your new type in the switch case
bool type_get_value_json(cJSON* value) 
{
    switch(type_int){
        case SWITCH_TYPE_INT: return switch_get_value_json(value);
        default: return default_get_value_json(value);
    }
    ESP_LOGE(TAG, "Error: type_get_value_json");
    return false;
}

// Add your new type in the switch case
bool type_set_value_json(cJSON* value)
{
    switch(type_int){
        case SWITCH_TYPE_INT: return switch_set_value_json(value);
        default: return default_set_value_json(value);
    }
    ESP_LOGE(TAG, "Error: type_set_value_json");
    return false;
}

// Add your new type in the switch case
bool type_pre_reboot(void)
{
    switch(type_int){
        case SWITCH_TYPE_INT: return switch_pre_reboot();
        default: return default_pre_reboot();
    }
    ESP_LOGE(TAG, "Error: type_pre_reboot");
    return false;
}