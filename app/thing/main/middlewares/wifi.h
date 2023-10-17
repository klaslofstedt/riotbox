/*
 * wifi.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _WIFI_H_
#define _WIFI_H_

#include "esp_wifi.h"

#define WIFI_SSID_MAX_LEN       32 + 1  // Max len acoording to spec + null terminator
#define WIFI_PWD_MAX_LEN        64 + 1  // Max len acoording to spec + null terminator
#define WIFI_MAX_SCAN_RESULTS   20      // TODO test with different sizes

typedef enum
{
    WIFI_HAS_SSID = BIT0,
    WIFI_HAS_PWD = BIT1,
} wifi_flags_t;

bool wifi_start_scan(void);
void wifi_stop_scan(void);
bool wifi_init(void);
bool wifi_stop(void);
bool wifi_reinit(void);
bool wifi_connect(void);
bool wifi_get_pwd(char* pwd);
bool wifi_set_pwd(const char* pwd);
bool wifi_get_ssid(char* ssid);
bool wifi_set_ssid(const char* ssid);
bool wifi_erase_credentials(void);
bool wifi_get_has_ssid(void);
bool wifi_get_has_pwd(void);
bool wifi_get_network_list(wifi_ap_record_t *network_list, uint16_t *list_max_size);


#endif /* _WIFI_H_ */
