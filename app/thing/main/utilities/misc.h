/*
 * misc.h
 *
 *  Created on: 31 dec 2021
 *      Author: klaslofstedt
 */

#ifndef _MISC_H_
#define _MISC_H_

#include <stdbool.h>

#define ID_SIZE               15

// Thing callback 
typedef void (*callback_t)(void);

bool hex_string_to_u8_array(char *in, uint8_t *out, size_t size_out);
void u8_array_to_ascii_string(const unsigned char *bytes, char *ascii_string, size_t size_in);
bool id_get(char *thing_id);

#endif /* _MISC_H_ */
