/*
 * ota.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _OTA_H_
#define _OTA_H_

typedef enum
{
    OTA_HAS_URL = BIT0,
} ota_flags_t;

bool ota_run(void);
bool ota_set_url(const char* json_str);

#endif /* _OTA_H_ */
