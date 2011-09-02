/*
 * endian.c - Network endianness helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "endian.h"

/**
 * htons - 16-bit host->network endianness conversion
 * @x: The value to convert
 */
uint16_t htons(uint16_t x)
{
    return (x << 8) | ((x >> 8) & 0xFF);
}

/**
 * ntohs - 16-bit network->host endianness conversion
 * @x: The value to convert
 */
uint16_t ntohs(uint16_t x)
{
    return htons(x);
}

/**
 * htonl - 32-bit host->network endianness conversion
 * @x: The value to convert
 */
uint32_t htonl(uint32_t x)
{
    uint32_t r;

    uint8_t *src = (uint8_t *)&x;
    uint8_t *dest = (uint8_t *)&r;

    dest[0] = src[3];
    dest[1] = src[2];
    dest[2] = src[1];
    dest[3] = src[0];

    return r;
}

/**
 * ntohl - 32-bit network->host endianness conversion
 * @x: The value to convert
 */
uint32_t ntohl(uint32_t x)
{
    return htonl(x);
}

/**
 * endian_convert - Convert endianness in a header/structure
 * @buffer: The address of the structure
 * @signature: The conversion signature
 * 
 * The signature is a bit mask such that a run of Y consecutive 1 bits
 * starting at bit no. X signifies the presence of a convertible field
 * of Y+1 bytes at byte offset X.
 */
void endian_convert(void *buffer, uint32_t signature)
{
    uint8_t curr_bit, prev_bit = 0, len = 0, offset, i, x;
    uint8_t *start = buffer;
    
    for (offset = 0;
        signature | prev_bit;
        signature >>= 1, offset++, prev_bit = curr_bit) {
        curr_bit = signature & 1;
        
        if (curr_bit) {
            if (!prev_bit) {
                start = ((uint8_t *)buffer) + offset;
                len = 1;
            }
            len++;
        } else if (prev_bit) {
            for (i = 0; i < (len >> 1); i++) {
                x = start[i];
                start[i] = start[len-i-1];
                start[len-i-1] = x;
            }
        }
    }
}
