/*
 * thing.h
 *
 *  Created on: 20 feb 2023
 *      Author: klaslofstedt
 */

#ifndef _THING_H_
#define _THING_H_

#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define THING_TYPE_NAME_MAX_SIZE    32
#define THING_HW_VERSION_MAX_SIZE   32

typedef enum
{
    THING_HAS_TYPE = BIT0,
    THING_HAS_HW_VERSION = BIT1,
} thing_flags_t;

bool thing_init(void);
bool thing_run(void);
bool thing_get_has_type(void);
bool thing_set_type(const char* type);
bool thing_get_has_hw_version(void);
bool thing_set_hw_version(const char* hw_version);
bool thing_reboot(void);

#endif /* _THING_H_ */
