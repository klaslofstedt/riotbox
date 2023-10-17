#include "lwip/err.h"
#include "lwip/sys.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "middlewares/wifi.h"
#include "drivers/storage.h"
#include "utilities/event.h"
#include <cJSON.h>

#define WIFI_STORAGE_KEY_FLAGS        "wifi_flags"
#define WIFI_STORAGE_KEY_SSID         "wifi_ssid"
#define WIFI_STORAGE_KEY_PWD          "wifi_pwd"

static const char *TAG = "WIFI";

static bool wifi_set_has_ssid(void);
static bool wifi_set_has_pwd(void);
static bool wifi_unset_has_ssid(void);
static bool wifi_unset_has_pwd(void);
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
static bool wifi_take_lock(int timeout_ms);
static void wifi_give_lock(void);

static SemaphoreHandle_t wifi_lock = NULL;
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;
static esp_netif_t *wifi_sta = NULL;


static bool wifi_set_has_ssid(void)
{
    return storage_set_flags(WIFI_STORAGE_KEY_FLAGS, WIFI_HAS_SSID);
}

static bool wifi_set_has_pwd(void)
{
    return storage_set_flags(WIFI_STORAGE_KEY_FLAGS, WIFI_HAS_PWD);
}

static bool wifi_unset_has_ssid(void)
{
    return storage_unset_flags(WIFI_STORAGE_KEY_FLAGS, WIFI_HAS_SSID);
}

static bool wifi_unset_has_pwd(void)
{
    return storage_unset_flags(WIFI_STORAGE_KEY_FLAGS, WIFI_HAS_PWD);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT){
        ESP_LOGI(TAG, "DEBUG WIFI EVENT: %ld", event_id);
    }
    if (event_base == IP_EVENT){
        ESP_LOGI(TAG, "DEBUG IP EVENT: %ld", event_id);
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "STA start...");
        event_trigger(EVENT_WIFI_START);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_give_lock();
        event_trigger(EVENT_WIFI_DISCONNECTED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_give_lock();
        event_trigger(EVENT_WIFI_CONNECTED);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        wifi_give_lock();
        event_trigger(EVENT_WIFI_CONNECTED);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE){
        event_trigger(EVENT_WIFI_SCAN_DONE);
    }
}

static bool wifi_take_lock(int timeout_ms)
{
    if (!(xSemaphoreTake(wifi_lock, timeout_ms / portTICK_PERIOD_MS) == pdTRUE)){
        ESP_LOGE(TAG, "Error: xSemaphoreTake");
        return false;
    }
    return true;
}

static void wifi_give_lock(void)
{
    xSemaphoreGive(wifi_lock);
    ESP_LOGI(TAG, "Give mutex ");
}

bool wifi_get_has_ssid(void)
{
    return storage_has_flags(WIFI_STORAGE_KEY_FLAGS, WIFI_HAS_SSID);
}

bool wifi_get_has_pwd(void)
{
    return storage_has_flags(WIFI_STORAGE_KEY_FLAGS, WIFI_HAS_PWD);
}

bool wifi_get_pwd(char *pwd)
{
    if (!wifi_get_has_pwd()){
        ESP_LOGE(TAG, "Error: wifi_get_has_pwd");
        return false;
    }
    return storage_get_blob(WIFI_STORAGE_KEY_PWD, pwd, WIFI_PWD_MAX_LEN);
}
bool wifi_set_pwd(const char* pwd)
{
    if (!storage_set_blob(WIFI_STORAGE_KEY_PWD, pwd, strlen(pwd) + 1)){
        ESP_LOGE(TAG, "Error: storage_set_blob");
        return false;
    }
    return wifi_set_has_pwd();
}

bool wifi_get_ssid(char *ssid)
{
    if (!wifi_get_has_ssid()){
        ESP_LOGE(TAG, "Error: wifi_get_has_ssid");
        return false;
    }
    return storage_get_blob(WIFI_STORAGE_KEY_SSID, ssid, WIFI_SSID_MAX_LEN);
}

bool wifi_set_ssid(const char* ssid)
{
    if (!storage_set_blob(WIFI_STORAGE_KEY_SSID, ssid, strlen(ssid) + 1)){
        ESP_LOGE(TAG, "Error: storage_set_blob");
        return false;
    }
    return wifi_set_has_ssid();
}

bool wifi_erase_credentials(void)
{
    if (!wifi_unset_has_ssid()){
        ESP_LOGE(TAG, "Error: wifi_unset_has_ssid");
        return false;
    }
    if (!wifi_unset_has_pwd()){
        ESP_LOGE(TAG, "Error: wifi_unset_has_pwd");
        return false;
    }
    return true;
}

bool wifi_start_scan(void)
{
    ESP_LOGI(TAG, "Trying to start scan");
    if (!wifi_take_lock(10000)){
        ESP_LOGE(TAG, "Error: wifi_take_lock");
        return false;
    }
    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true
    };
    ESP_LOGI(TAG, "Scanning...");
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    return true;
}

void wifi_stop_scan(void)
{
    esp_wifi_scan_stop();
    wifi_give_lock();
}

bool wifi_get_network_list(wifi_ap_record_t *network_list, uint16_t *list_max_size) 
{
    esp_err_t err = esp_wifi_scan_get_ap_records(list_max_size, network_list);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: Failed to get AP records; error=%d", err);
        return false;
    }

    for (int i = 0; i < *list_max_size; i++){
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d", network_list[i].ssid, network_list[i].rssi);
    }

    wifi_stop_scan();
    return true;
}

bool wifi_init(void)
{
    ESP_LOGI(TAG, "Initialise");
    esp_err_t err;
    wifi_lock = xSemaphoreCreateBinary();
    wifi_give_lock();

    err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_netif_init");
        return false;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_event_loop_create_default");
        return false;
    }
    wifi_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_wifi_init");
        return false;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT,
                                              ESP_EVENT_ANY_ID,
                                              &wifi_event_handler,
                                              NULL,
                                              &instance_any_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_event_handler_instance_register");
        return false;
    }

    err = esp_event_handler_instance_register(IP_EVENT,
                                              IP_EVENT_STA_GOT_IP,
                                              &wifi_event_handler,
                                              NULL,
                                              &instance_got_ip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_event_handler_instance_register");
        return false;
    }

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_wifi_set_mode");
        return false;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "dummyssid",
            .password = "dummypassword",
            .threshold.authmode = WIFI_AUTH_OPEN,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    if (wifi_get_has_ssid() && wifi_get_has_pwd()){
        char wifi_ssid[WIFI_SSID_MAX_LEN];
        char wifi_pwd[WIFI_PWD_MAX_LEN];
        
        
        if (wifi_get_ssid(wifi_ssid)){
            memcpy(wifi_config.sta.ssid, wifi_ssid, strlen(wifi_ssid) + 1);
        }
        if (wifi_get_pwd(wifi_pwd)){
            memcpy(wifi_config.sta.password, wifi_pwd, strlen(wifi_pwd) + 1);
        }
    } else {
        ESP_LOGE(TAG, "Error: wifi_get_has_ssid && wifi_get_has_pwd");
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_wifi_set_config");
        return false;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_wifi_start");
        return false;
    }

    return true;
}

bool wifi_reinit(void)
{
    if (!wifi_stop()){
        ESP_LOGE(TAG, "Error: wifi_stop");
        return false;
    }
    if (!wifi_init()){
        ESP_LOGE(TAG, "Error: wifi_init");
        return false;
    }
    return true;
}

bool wifi_stop(void)
{
    ESP_LOGI(TAG, "Deinitialising");
    esp_err_t err;
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
    err = esp_wifi_stop();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_wifi_stop");
        return false;
    }
    err = esp_wifi_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_wifi_deinit");
        return false;
    }
    esp_netif_destroy(esp_netif_get_handle_from_ifkey("WIFI_STA"));
    esp_event_loop_delete_default();
    esp_netif_deinit();
    esp_netif_destroy(wifi_sta);
    vSemaphoreDelete(wifi_lock);

    return true;
}

bool wifi_connect(void)
{
    ESP_LOGI(TAG, "Trying to connect");
    if (!wifi_take_lock(0)){
        ESP_LOGE(TAG, "Error: wifi_take_lock");
        return false;
    }
    ESP_LOGI(TAG, "Connecting...");
    esp_wifi_connect();
    return true;
}
