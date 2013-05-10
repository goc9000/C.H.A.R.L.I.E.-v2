/*
 * misc.c - Miscellaneous utilities
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */
 
#include "string.h"
 
#include "misc.h"
#include "bool.h"

/**
 * itoa10 - Renders a 16-bit integer as a string (simplified version of itoa())
 * @value: The integer value to render as a string
 * @buf: A buffer to receive the representation of the int.
 * 
 * Returns a pointer to the start of the string.
 */
char* itoa10(int16_t value, char *buf)
{
    return ltoa10((int32_t)value, buf);
}

/**
 * ltoa10 - Renders a 32-bit integer as a string (simplified version of ltoa())
 * @value: The integer value to render as a string
 * @buf: A buffer to receive the representation of the int.
 * 
 * Returns a pointer to the start of the string.
 */
char* ltoa10(int32_t value, char *buf)
{
    bool negative = FALSE;
    char *ptr = buf;
    
    if (value < 0) {
        negative = TRUE;
        value = -value;
    }
    
    do {
        *(ptr++) = '0' + (value % 10);
        value /= 10;
    } while (value);
    
    if (negative)
        *(ptr++) = '-';
    
    *ptr = 0;
    
    return strrev(buf);
}
