/*
 * misc.h - Miscellaneous utilities
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _MISC_H
#define _MISC_H

#include <inttypes.h>

char* itoa10(int16_t value, char *buf);
char* ltoa10(int32_t value, char *buf);

static inline int32_t clamp(int32_t value, int32_t min_val, int32_t max_val)
{
    return (value < min_val)
        ? min_val
        : ((value > max_val) ? max_val : value);
}

static inline int32_t max(int32_t a, int32_t b)
{
    return (a > b) ? a : b;
}

static inline int32_t min(int32_t a, int32_t b)
{
    return (a < b) ? a : b;
}

#endif
