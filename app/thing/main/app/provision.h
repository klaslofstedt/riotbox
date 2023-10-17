/*
 * provision.h
 *
 *  Created on: 24 jun 2023
 *      Author: klaslofstedt
 */
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <inttypes.h>

#ifndef _PROVISION_H_
#define _PROVISION_H_

bool provision_run(void);
bool provision_init(void);

#endif /* _PROVISION_H_ */
