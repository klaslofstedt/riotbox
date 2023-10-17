/*
 * nvs.h
 *
 *  Created on: 20 jan 2023
 *      Author: klaslofstedt
 */

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <stdbool.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

bool storage_has_flags(const char* key, uint32_t flags);
bool storage_set_flags(const char* key, uint32_t flags);
bool storage_unset_flags(const char* key, uint32_t flags);
bool storage_erase_blob(const char* key);
bool storage_set_blob(const char* key, const char* buffer, size_t size);
bool storage_get_blob(const char* key, char* buffer, size_t size);
bool storage_append_blob(const char* key, const char* buffer, size_t max_size);
bool storage_init(void);

#endif /* _STORAGE_H_ */
