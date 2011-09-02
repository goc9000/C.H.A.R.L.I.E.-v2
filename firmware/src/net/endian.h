/*
 * endian.h - Network endianness helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _ENDIAN_H
#define _ENDIAN_H

#include <inttypes.h>
#include <avr/io.h>

#define DEF_EC_FIELD(_struc,_name,_len) ( \
    (1UL << (__builtin_offsetof(_struc,_name) + _len - 1)) - \
    (1UL << __builtin_offsetof(_struc,_name)) \
    )

uint16_t htons(uint16_t x);
uint16_t ntohs(uint16_t x);
uint32_t htonl(uint32_t x);
uint32_t ntohl(uint32_t x);
void endian_convert(void *buffer, uint32_t signature);

#endif
