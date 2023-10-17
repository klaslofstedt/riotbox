/*
 * auth.h
 *
 *  Created on: 17 sep 2023
 *      Author: klaslofstedt
 */

#ifndef _AUTH_H_
#define _AUTH_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef enum
{
    AUTH_USE_OTA = BIT0,
    AUTH_USE_PROVISION = BIT1,
} auth_flags_t;

typedef enum
{
    AUTH_OTA,
    AUTH_PROVISION,
} auth_t;

bool auth_init(void);
bool auth_use_ota(void);
bool auth_use_provision(void);
auth_t auth_get_which(void);

#endif /* _AUTH_H_ */
