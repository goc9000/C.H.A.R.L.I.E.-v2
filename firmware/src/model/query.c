/*
 * query.c - Query support for files with date-ordered records
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/io.h>
#include <string.h>

#include "query.h"

static err_t _query_init(Query *query, uint8_t item_size);
static void _query_filter(Query *query, time_t time, bool before);

/**
 * query_make - Creates a query on a file
 * @query: The query object to be initialized
 * @filename: The name of the file
 * @item_size: The size of an item in the file, in bytes
 */
err_t query_make(Query* query, const char *filename, err_t item_size)
{
    err_t stat = file_open(&query->file, filename);
    
    if (stat) {
        return stat;
    }
    
    return _query_init(query, item_size);
}

/**
 * query_make_P - Creates a query on a file, program memory version
 * @query: The query object to be initialized
 * @filename: The name of the file, stored in program memory
 * @item_size: The size of an item in the file, in bytes
 */
err_t query_make_P(Query *query, PGM_P filename, uint8_t item_size)
{
    err_t stat = file_open_P(&query->file, filename);
    
    if (stat) {
        return stat;
    }
    
    return _query_init(query, item_size);
}

static err_t _query_init(Query *query, uint8_t item_size) {
    query->position = 0;
    query->origin = 0;
    query->count = file_size(&query->file) / item_size;
    query->item_size = item_size;
    query->reverse = FALSE;
    
    return E_OK;
}

/**
 * query_reverse - Reverses the order of entries in a query
 * @query: The query object to be modified
 */
void query_reverse(Query *query)
{
    query->origin = query->reverse ? query->origin-query->count+1
        : query->origin+query->count-1;
    query->reverse ^= 1;
}

/**
 * query_limit - Limits the number of entries in a query
 * @query: The query object to be modified
 * @limit: The maximum number of entries left after the current position
 */
void query_limit(Query *query, uint32_t limit)
{
    if (query->count > query->position+limit) {
        query->count = query->position+limit;
    }
}

/**
 * query_get - Gets the element at the current position
 * @query: A query object
 * @item: A buffer to store the element in
 * 
 * Returns TRUE if the item was loaded, FALSE if there are no items left in
 * the query.
 */
bool query_get(Query *query, void *item)
{
    uint32_t abs_posn;
    
    if (query->position == query->count) {
        return FALSE;
    }

    abs_posn = query->reverse ? query->origin-query->position
        : query->origin+query->position;

    file_seek(&query->file, abs_posn * query->item_size);
    file_read(&query->file, item, query->item_size);

    return TRUE;
}

/**
 * query_get_time - Gets the time at the current position
 * @query: A query object
 * @time: A variable to store the time in
 * 
 * Returns TRUE if the time was loaded, FALSE if there are no items left in
 * the query.
 */
bool query_get_time(Query *query, time_t *time)
{
    uint32_t abs_posn;
    
    if (query->position == query->count) {
        return FALSE;
    }

    abs_posn = query->reverse ? query->origin-query->position
        : query->origin+query->position;

    file_seek(&query->file, abs_posn * query->item_size);
    file_read(&query->file, time, sizeof(time_t));

    return TRUE;
}

/**
 * query_next - Advances to the next item
 * @query: The query object to be modified
 */
void query_next(Query *query)
{
    query_skip(query, 1);
}

/**
 * query_skip - Skips over a number of items
 * @query: The query object to be modified
 * @amount: The number of items to skip
 */
void query_skip(Query *query, uint32_t amount)
{
    query->position += amount;
    if (query->position > query->count) {
        query->position = query->count;
    }
}

/**
 * query_last - Moves to the last item in the query
 * @query: The query object to be modified
 */
void query_last(Query *query)
{
    if (query->count) {
        query->position = query->count-1;
    }
}

/**
 * query_paginate - Selects a page in a query
 * @query: The query object to be modified
 * @page_size: The number of items in a page
 * @page: A variable that stores the selected page
 * @num_pages: A variable for storing the number of pages
 */
void query_paginate(Query *query, uint16_t page_size, uint16_t *page,
    uint16_t *num_pages)
{
    *num_pages = (query->count + page_size-1) / page_size;
    if (!*num_pages) {
        *num_pages = 1;
    }

    if (!*page) {
        *page = 1;
    }
    if (*page > *num_pages) {
        *page = *num_pages;
    }
    
    query_skip(query, page_size * (*page-1));
    query_limit(query, page_size);
}

/**
 * query_filter_before - Keeps only items occuring before (or at) a given time
 * @query: The query object to be modified
 * @time: A moment in time
 */
void query_filter_before(Query *query, time_t time)
{
    _query_filter(query, time, TRUE);
}

/**
 * query_filter_after - Keeps only items occuring after (or at) a given time
 * @query: The query object to be modified
 * @time: A moment in time
 */
void query_filter_after(Query *query, time_t time)
{
    _query_filter(query, time, FALSE);
}

static void _query_filter(Query *query, time_t time, bool before)
{
    uint32_t pivot;
    time_t piv_time;
    
    uint32_t new_first = query->reverse ? query->origin-query->count+1 : query->origin;
    uint32_t new_last = query->reverse ? query->origin+1 : query->origin+query->count;

    uint32_t first = new_first;
    uint32_t last = new_last;

    while (first != last) {
        pivot = (first+last) >> 1;
        
        file_seek(&query->file, pivot * query->item_size);
        file_read(&query->file, (uint8_t*)&piv_time, sizeof(time_t));

        if (before) {
            if (piv_time > time) {
                last = pivot;
                new_last = pivot;
            } else {
                first = pivot+1;
            }
        } else {
            if (piv_time < time) {
                first = pivot+1;
                new_first = pivot+1;
            } else {
                last = pivot;
            }
        }
    }

    query->origin = query->reverse ? new_last-1 : new_first;
    query->count = new_last - new_first;
}

/**
 * query_tell - Gets the current position in a query
 * @query: A query object
 * 
 * The position of the first element is 0.
 */
uint32_t query_tell(const Query *query)
{
    return query->position;
}

/**
 * query_count - Gets the total number of items in a query
 * @query: A query object
 */
uint32_t query_count(const Query *query)
{
    return query->count;
}

/**
 * query_remaining - Gets the number of items remaining in a query
 * @query: A query object
 * 
 * Note that the count includes the current element.
 */
uint32_t query_remaining(const Query *query)
{
    return query->count - query->position;
}
