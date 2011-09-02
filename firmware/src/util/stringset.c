/*
 * stringset.c - String set utilities
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "stringset.h"

/**
 * strset_get - Gets the Nth string in a string set
 * @string_set: The string set
 * @index: The index of the string to select
 */
PGM_P strset_get(PGM_P string_set, uint8_t index)
{
    while (index--) {
        string_set += strlen_P(string_set) + 1;
    }
    
    return string_set;
}

/**
 * strset_find - Searches for a string in a string set
 * @string_set: The string set
 * @count: The number of entries to search
 * @needle: The string to search for
 * 
 * Returns the index of the string in the string set, or 'count' if it
 * was not found.
 */
uint8_t strset_find(PGM_P string_set, uint8_t count, const char *needle)
{
    uint8_t i;
    
    for (i = 0; i < count; i++) {
        if (!strcmp_P(needle, string_set)) {
            return i;
        }
        
        string_set += strlen_P(string_set) + 1;
    }
    
    return count;
}
