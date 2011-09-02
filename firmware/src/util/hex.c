/*
 * hex.c - Hexadecimal utilities
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */
 
#include <ctype.h>

#include "hex.h"

/**
 * hex_parse - Parses a hex string.
 * @input: The string to parse
 * @max_digits: The maximum number of hex digits to parse
 * @output: A variable to hold the output
 * 
 * Returns the number of digits parsed.
 */
uint8_t hex_parse(const char *input, uint8_t max_digits, uint32_t *output)
{
    uint8_t digits = 0;
    char c;

    *output = 0;
    while (digits < max_digits) {
        c = toupper(*(input++));
        if (!isdigit(c) && !((c >= 'A') && (c <= 'F'))) {
            break;
        }
        
        *output = (*output << 4) +
            (isdigit(c) ? (c - '0') : (c + 10 - 'A'));
        digits++;
    }
    
    return digits;
}

/**
 * hex_format - Formats a fixed-width hex string into a buffer.
 * @buf: A buffer to receive the string. Must be at least digits+1 bytes wide.
 * @digits: The number of digits to print
 * @value: The value to display
 */
void hex_format(char* buf, uint8_t digits, uint32_t value)
{
    uint8_t nib;
    
    buf += digits;
    
    *(buf--) = 0;
    while (digits--) {
        nib = (uint8_t)(value & 15);
        *(buf--) = (nib < 10) ? ('0' + nib) : ('A' - 10 + nib);
        value >>= 4;
    }
}
