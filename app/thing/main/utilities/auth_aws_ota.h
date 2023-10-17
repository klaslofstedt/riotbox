/*
 * auth_ota_aws.h
 *
 *  Created on: 17 sep 2023
 *      Author: klaslofstedt
 */

#ifndef _AUTH_AWS_OTA_H_
#define _AUTH_AWS_OTA_H_

extern const char auth_aws_ota_root_ca[];
extern const char auth_aws_ota_thing_cert[];
extern const char auth_aws_ota_thing_key[];

#define PEM_AWS_ROOT_CA_SIZE (sizeof(auth_aws_ota_root_ca) / sizeof(auth_aws_ota_root_ca[0]))
#define PEM_AWS_THING_CERTIFICATE_SIZE (sizeof(auth_aws_ota_thing_cert) / sizeof(auth_aws_ota_thing_cert[0]))
#define PEM_AWS_THING_KEY_SIZE (sizeof(auth_aws_ota_thing_key) / sizeof(auth_aws_ota_thing_key[0]))

#endif /* _AUTH_AWS_OTA_H_ */