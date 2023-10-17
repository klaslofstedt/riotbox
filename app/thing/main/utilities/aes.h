/*
 * aes.h
 *
 *  Created on: 1 feb 2023
 *      Author: klaslofstedt
 */

#ifndef _AES_H_
#define _AES_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define AES_BLOCK_SIZE            16


typedef enum
{
    AES_HAS_KEY = BIT0,
} aes_flags_t;

bool aes_init();
bool aes_deinit();
bool aes_set_key(const char *aes);
bool aes_get_key(char *aes);
bool aes_get_has_key(void);

bool aes_crypto(char *encrypted_string, char *decrypted_string, size_t size);


#endif /* _AES_H_ */
