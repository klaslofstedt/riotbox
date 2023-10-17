/*
 * switch.h
 *
 *  Created on: 23 feb 2023
 *      Author: klaslofstedt
 */

#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <stdbool.h>
#include <stdio.h>
#include <cJSON.h>
#include "utilities/misc.h"


#define SWITCH_TYPE_STR "SWITCH"
#define SWITCH_TYPE_INT 1


typedef struct switch_readwrite_t {
    bool status;
} switch_readwrite_t;

typedef struct switch_read_t {

} switch_read_t;

typedef struct switch_value_t {
    switch_readwrite_t readwrite;
    switch_read_t read;
} switch_value_t;


bool switch_get_value_json(cJSON *value);
bool switch_set_value_json(cJSON *value);
bool switch_init(callback_t callback_publish_value);
bool switch_pre_reboot(void);

#endif /* _SWITCH_H_ */
