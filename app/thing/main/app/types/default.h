/*
 * default.h
 *
 *  Created on: 5 oct 2023
 *      Author: klaslofstedt
 */

#ifndef _DEFAULT_H_
#define _DEFAULT_H_

#include <stdbool.h>
#include <stdio.h>
#include <cJSON.h>
#include "utilities/misc.h"


#define DEFAULT_TYPE_STR "DEFAULT"
#define DEFAULT_TYPE_INT 0


typedef struct default_readwrite_t {
    /* Developer: Add your read-write properties here */
} default_readwrite_t;

typedef struct default_read_t {
    /* Developer: Add your read-only properties here */
} default_read_t;

typedef struct default_value_t {
    default_readwrite_t readwrite;
    default_read_t read;
} default_value_t;


bool default_get_value_json(cJSON *value);
bool default_set_value_json(cJSON *value);
bool default_init(callback_t callback_publish_value);
bool default_pre_reboot(void);

#endif /* _DEFAULT_H_ */
