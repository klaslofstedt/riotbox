/*
 * thing.h
 *
 *  Created on: 21 jul 2023
 *      Author: klaslofstedt
 */

#ifndef _MOBILE_H_
#define _MOBILE_H_

#define MOBILE_STRING_MAX_LEN 16

typedef struct mobile_readwrite_t
{
    char network[MOBILE_STRING_MAX_LEN];
} mobile_readwrite_t;

typedef struct mobile_read_t
{
    char nickname[MOBILE_STRING_MAX_LEN];
    char sw_version[MOBILE_STRING_MAX_LEN];
} mobile_read_t;

typedef struct mobile_value_t
{
    mobile_readwrite_t readwrite;
    mobile_read_t read;
} mobile_value_t;

bool mobile_set_value_json(const char* json_str);
bool mobile_get_value_json(cJSON* root);
bool mobile_init(void);


#endif /* _MOBILE_H_ */
