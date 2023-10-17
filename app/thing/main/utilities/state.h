/*
 * state.h
 *
 *  Created on: 20 jun 2023
 *      Author: klaslofstedt
 */

#ifndef _STATE_H_
#define _STATE_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

typedef enum
{
    STATE_UNINITIALISED = BIT0,
    STATE_PROVISION = BIT1,
    STATE_OTA = BIT2,
    STATE_THING = BIT3,
} states_t;

void state_set(states_t state);
states_t state_get(void);
const char* state_string(states_t state);
bool state_init(void);

#endif /* _STATE_H_ */
