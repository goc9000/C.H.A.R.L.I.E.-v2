/*
 * stringset.h - String set utilities
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _STRINGSET_H
#define _STRINGSET_H

#include <inttypes.h>
#include <avr/pgmspace.h>

PGM_P strset_get(PGM_P string_set, uint8_t index);
uint8_t strset_find(PGM_P string_set, uint8_t count, const char *needle);

#endif
