/*
 * aws.h
 *
 *  Created on: 17 sep 2023
 *      Author: klaslofstedt
 */

#ifndef _AUTH_AWS_PROVISION_H_
#define _AUTH_AWS_PROVISION_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <inttypes.h>

// 256 * 19 = 4864 bytes. NVS size is 0x4000 = 16384 bytes
#define AUTH_AWS_PROVISION_ROOT_CA_BUFFER_SIZE 1280    // 256 * 5
#define AUTH_AWS_PROVISION_THING_CERT_BUFFER_SIZE 1536 // 256 * 6
#define AUTH_AWS_PROVISION_THING_KEY_BUFFER_SIZE 2048  // 256 * 8

typedef enum
{
    AUTH_AWS_PROVISION_HAS_ROOT_CA = BIT0,
    AUTH_AWS_PROVISION_HAS_THING_CERT = BIT1,
    AUTH_AWS_PROVISION_HAS_THING_KEY = BIT2,
} auth_aws_provision_flags_t;

extern char auth_aws_provision_root_ca[AUTH_AWS_PROVISION_ROOT_CA_BUFFER_SIZE];
extern char auth_aws_provision_thing_cert[AUTH_AWS_PROVISION_THING_CERT_BUFFER_SIZE];
extern char auth_aws_provision_thing_key[AUTH_AWS_PROVISION_THING_KEY_BUFFER_SIZE];

bool auth_aws_provision_load_thing_cert(void);
bool auth_aws_provision_append_thing_cert(const char *blob);
bool auth_aws_provision_erase_thing_cert(void);

bool auth_aws_provision_load_thing_key(void);
bool auth_aws_provision_append_thing_key(const char *blob);
bool auth_aws_provision_erase_thing_key(void);

bool auth_aws_provision_load_root_ca(void);
bool auth_aws_provision_append_root_ca(const char *blob);
bool auth_aws_provision_erase_root_ca(void);

bool auth_aws_provision_get_has_root_ca(void);
bool auth_aws_provision_get_has_thing_cert(void);
bool auth_aws_provision_get_has_thing_key(void);

bool auth_aws_provision_set_has_root_ca(void);
bool auth_aws_provision_set_has_thing_cert(void);
bool auth_aws_provision_set_has_thing_key(void);

bool auth_aws_provision_unset_has_root_ca(void);
bool auth_aws_provision_unset_has_thing_cert(void);
bool auth_aws_provision_unset_has_thing_key(void);

#endif /* _AUTH_AWS_PROVISION_H_ */
