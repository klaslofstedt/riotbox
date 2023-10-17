#include "app/types/switch.h"
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

#define SWITCH_BUTTON_DEBOUNCE_MS   200

static const char *TAG = "SWITCH";

static SemaphoreHandle_t switch_value_lock = NULL;
static SemaphoreHandle_t switch_button_trigger = NULL;

static callback_t switch_publish_value = NULL;

static switch_value_t switch_value_g = {
    .read = {
    },
    .readwrite = {
        .status = false,
    }
};

static void switch_task(void* arg);
static bool switch_get_struct(switch_value_t *switch_value);
static bool switch_set_struct(switch_value_t switch_value);
static bool switch_take_value_lock(void);
static void switch_give_value_lock(void);
static bool switch_init_button(void);
static bool switch_init_led(void);
static void switch_do_button_trigger(void);
static bool switch_wait_button_trigger(void);
static bool switch_update_hw(void);

static bool switch_take_value_lock(void)
{
    if (!(xSemaphoreTake(switch_value_lock, 0) == pdTRUE)){
        ESP_LOGE(TAG, "Error: xSemaphoreTake");
        return false;
    }
    return true;
}

static void switch_give_value_lock(void)
{
    xSemaphoreGive(switch_value_lock);
}

static bool switch_wait_button_trigger(void)
{
    if (!(xSemaphoreTake(switch_button_trigger, portMAX_DELAY) == pdTRUE)){
        ESP_LOGE(TAG, "Error: xSemaphoreTake");
        return false;
    }
    return true;
}

static void switch_do_button_trigger(void)
{
    xSemaphoreGiveFromISR(switch_button_trigger, NULL);
}

static bool switch_get_struct(switch_value_t *switch_value) 
{    
    if(!switch_take_value_lock()) {
        ESP_LOGE(TAG, "Error: switch_take_value_lock");
        return false;
    }
    *switch_value = switch_value_g;
    switch_give_value_lock();
    return true;
}

static bool switch_set_struct(switch_value_t switch_value) 
{
    if(!switch_take_value_lock()) {
        ESP_LOGE(TAG, "Error: switch_take_value_lock");
        return false;
    }
    switch_value_g.readwrite.status = switch_value.readwrite.status;
    switch_give_value_lock();
    return true;
}

static void IRAM_ATTR switch_button_isr_handler(void* arg)
{
    static TickType_t last_time = 0;
    TickType_t now_time = xTaskGetTickCountFromISR();

    if (now_time - last_time > pdMS_TO_TICKS(SWITCH_BUTTON_DEBOUNCE_MS)){
        switch_do_button_trigger();
    }
    last_time = now_time;
}

static bool switch_init_button(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = 1ULL << THING_BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    if (gpio_config(&button_config) != ESP_OK) {
        ESP_LOGE(TAG, "Error: gpio_config");
        return false;
    }

    if (gpio_install_isr_service(0) != ESP_OK) {
        ESP_LOGE(TAG, "Error: gpio_install_isr_service");
        return false;
    }

    if(gpio_isr_handler_add(THING_BUTTON_GPIO, switch_button_isr_handler, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "Error: gpio_isr_handler_add");
        return false;
    }

    return true;
}

static bool switch_init_led(void)
{
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << THING_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };

    if(gpio_config(&led_config) != ESP_OK) {
        ESP_LOGE(TAG, "Error: gpio_config");
        return false;
    }

    gpio_set_level(THING_LED_GPIO, 0);

    return true;
}

static void switch_task(void* arg) 
{
    switch_value_t switch_value;
    if (switch_get_struct(&switch_value)){
        switch_update_hw();
    }

    while(true) {
        if(switch_wait_button_trigger()){
            switch_value.readwrite.status = !switch_value.readwrite.status;
            if (switch_set_struct(switch_value)){
                switch_update_hw();
                // Trigger MQTT publish of switch_value
                switch_publish_value();
            }
        }
    }
}

static bool switch_update_hw(void)
{   
    switch_value_t switch_value;
    if (!switch_get_struct(&switch_value)) {
        ESP_LOGE(TAG, "Error: switch_get_struct");
        return false;
    } else {
        gpio_set_level(THING_LED_GPIO, switch_value.readwrite.status);
    }

    return true;
}

bool switch_set_value_json(cJSON* value)
{
    cJSON *read = cJSON_GetObjectItem(value, "read");
    cJSON *readwrite = cJSON_GetObjectItem(value, "readwrite");
    if (!read || !readwrite) {
        ESP_LOGE(TAG, "Error: !read || !readwrite");
        return false;
    }

    cJSON *status = cJSON_GetObjectItem(readwrite, "status");
    if (!cJSON_IsBool(status)) {
        ESP_LOGE(TAG, "Error: cJSON_IsBool");
        return false;
    }

    // Update the global thing struct thread safely
    switch_value_t switch_value;
    if (!switch_get_struct(&switch_value)) {
        ESP_LOGE(TAG, "Error: switch_get_struct");
        return false;
    }
    switch_value.readwrite.status = cJSON_IsTrue(status);
    if (!switch_set_struct(switch_value)) {
        ESP_LOGE(TAG, "Error: switch_set_struct");
        return false;
    }

    switch_update_hw();

    return true;
}

bool switch_get_value_json(cJSON* value)
{
    switch_value_t switch_value;
    if (!switch_get_struct(&switch_value)){
        ESP_LOGE(TAG, "Error: switch_get_struct");
        return false;
    }

    cJSON *read = cJSON_GetObjectItem(value, "read");
    cJSON *readwrite = cJSON_GetObjectItem(value, "readwrite");
    if (!read || !readwrite) {
        ESP_LOGE(TAG, "Error: !read || !readwrite");
        return false;
    }
    cJSON_AddBoolToObject(readwrite, "status", switch_value.readwrite.status);

    return true;
}

bool switch_pre_reboot(void)
{
    return true;
}

bool switch_init(callback_t callback_publish_value)
{
    ESP_LOGI(TAG, "Initialise");
    // Create a callback to trigger MQTT publish
    switch_publish_value = callback_publish_value;

    switch_init_button();
    switch_init_led();

    // Create and give switch struct mutex
    switch_value_lock = xSemaphoreCreateBinary();
    switch_button_trigger = xSemaphoreCreateBinary();
    switch_give_value_lock();

    if (xTaskCreate(switch_task, "switch_task", 2048, NULL, 5, NULL) != pdTRUE){
        ESP_LOGE(TAG, "Error: xTaskCreate");
        return false;
    }
    return true;
}
