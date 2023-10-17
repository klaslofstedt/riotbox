/*
 * type.h
 *
 *  Created on: 10 sep 2023
 *      Author: klaslofstedt
 */

#ifndef _TYPE_H_
#define _TYPE_H_

#include <stdbool.h>
#include <stdio.h>
#include <cJSON.h>
#include "utilities/misc.h"

bool type_set_int(const char *type_str);
bool type_init(callback_t callback_publish_value);
bool type_get_value_json(cJSON *value);
bool type_set_value_json(cJSON *value);
bool type_pre_reboot(void);

#endif /* _TYPE_H_ */