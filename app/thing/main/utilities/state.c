/*
 * state.c
 *
 *  Created on: 20 jun 2023
 *      Author: klaslofstedt
 */

#include "utilities/state.h"
#include "esp_log.h"

static const char *TAG = "STATE";

static states_t state_current;

const char* state_string(states_t state)
{
    switch (state) {
        case STATE_UNINITIALISED: return "STATE_UNINITIALISED";
        case STATE_PROVISION: return "STATE_PROVISION";
        case STATE_OTA: return "STATE_OTA";
        case STATE_THING: return "STATE_THING";
        default: return "STATE_UNKNOWN";
    }
}

void state_set(states_t state)
{
    state_current = state;
    ESP_LOGI(TAG, "%s: ", state_string(state_current));
}

states_t state_get(void)
{
    return state_current;
}

bool state_init(void)
{
    ESP_LOGI(TAG, "Initialise");
    state_set(STATE_UNINITIALISED);
    return true;
}

