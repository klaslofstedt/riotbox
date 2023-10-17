#include "app/types/default.h"
#include <cJSON.h>
#include <stdbool.h>
#include <string.h>
#include "utilities/misc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "board/board.h"

static const char *TAG = "DEFAULT";

static SemaphoreHandle_t default_value_lock = NULL;

static callback_t default_publish_value = NULL;

static default_value_t default_value_g = {
    .read = {
        /* Developer: Initialise your read-only properties here */
    },
    .readwrite = {
        /* Developer: Initialise your read-write properties here */
    }
};

static void default_task(void* arg);
static bool default_get_struct(default_value_t *default_value);
static bool default_set_struct(default_value_t default_value);
static bool default_take_value_lock(void);
static void default_give_value_lock(void);
static bool default_update_hw(void);

static bool default_take_value_lock(void)
{
    if (!(xSemaphoreTake(default_value_lock, 0) == pdTRUE)){
        ESP_LOGE(TAG, "Error: xSemaphoreTake");
        return false;
    }
    return true;
}

static void default_give_value_lock(void)
{
    xSemaphoreGive(default_value_lock);
}

static bool default_get_struct(default_value_t *default_value) 
{    
    if(!default_take_value_lock()) {
        ESP_LOGE(TAG, "Error: default_take_value_lock");
        return false;
    }
    *default_value = default_value_g;
    default_give_value_lock();
    return true;
}

static bool default_set_struct(default_value_t default_value) 
{
    if(!default_take_value_lock()) {
        ESP_LOGE(TAG, "Error: default_take_value_lock");
        return false;
    }
    default_value_g.readwrite = default_value.readwrite;
    default_give_value_lock();
    return true;
}

static void default_task(void* arg) 
{
    default_value_t default_value;
    if (default_get_struct(&default_value)){
        default_update_hw();
    }

    while(true) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Publish default value to mobile app");
        default_publish_value();
    }
}

static bool default_update_hw(void)
{   
    default_value_t default_value;
    if (!default_get_struct(&default_value)) {
        ESP_LOGE(TAG, "Error: default_get_struct");
        return false;
    }

    /* Developer: Update your hardware here */

    return true;
}

bool default_set_value_json(cJSON* value)
{
    default_value_t default_value;
    if (!default_get_struct(&default_value)) {
        ESP_LOGE(TAG, "Error: default_get_struct");
        return false;
    }

    cJSON *read = cJSON_GetObjectItem(value, "read");
    cJSON *readwrite = cJSON_GetObjectItem(value, "readwrite");
    if (!read || !readwrite) {
        ESP_LOGE(TAG, "Error: !read || !readwrite");
        return false;
    }

    /* Developer: Parse your JSON properties here */

    /* Developer: Update default_value properties here */

    if (!default_set_struct(default_value)) {
        ESP_LOGE(TAG, "Error: default_set_struct");
        return false;
    }

    default_update_hw();

    return true;
}

bool default_get_value_json(cJSON* value)
{
    default_value_t default_value;
    if (!default_get_struct(&default_value)){
        ESP_LOGE(TAG, "Error: default_get_struct");
        return false;
    }

    cJSON *read = cJSON_GetObjectItem(value, "read");
    cJSON *readwrite = cJSON_GetObjectItem(value, "readwrite");
    if (!read || !readwrite) {
        ESP_LOGE(TAG, "Error: !read || !readwrite");
        return false;
    }

    /* Developer: Add your properties to JSON object here */

    return true;
}

bool default_pre_reboot(void)
{
    /* Developer: If you want to do anything before rebooting */
    return true;
}

bool default_init(callback_t callback_publish_value)
{
    ESP_LOGI(TAG, "Initialise");
    // Create a callback to trigger MQTT publish
    default_publish_value = callback_publish_value;

    // Create and give default struct mutex
    default_value_lock = xSemaphoreCreateBinary();

    default_give_value_lock();

    if (xTaskCreate(default_task, "default_task", 2048, NULL, 5, NULL) != pdTRUE){
        ESP_LOGE(TAG, "Error: xTaskCreate");
        return false;
    }
    return true;
}
