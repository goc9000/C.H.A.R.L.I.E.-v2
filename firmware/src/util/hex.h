/*
 * hex.h - Hexadecimal utilities
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _HEX_H
#define _HEX_H

#include <inttypes.h>

#include "util/bool.h"

uint8_t hex_parse(const char *input, uint8_t max_digits, uint32_t *output);
void hex_format(char* buf, uint8_t digits, uint32_t value);

#endif
