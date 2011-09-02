/*
 * query.h - Query support for files with date-ordered records
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _QUERY_H
#define _QUERY_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#include "time/time.h"
#include "storage/filesys.h"
#include "util/error.h"
#include "util/bool.h"

typedef struct {
    file_t file;
    uint32_t position;
    uint32_t origin;
    uint32_t count;
    uint16_t item_size;
    bool reverse : 1;
} Query;

err_t query_make(Query *query, const char *filename, uint8_t item_size);
err_t query_make_P(Query *query, PGM_P filename, uint8_t item_size);
void query_reverse(Query *query);
void query_limit(Query *query, uint32_t limit);
bool query_get(Query *query, void *item);
bool query_get_time(Query *query, time_t *time);
void query_next(Query *query);
void query_skip(Query *query, uint32_t amount);
void query_last(Query *query);
void query_paginate(Query *query, uint16_t page_size,
    uint16_t *page, uint16_t *num_pages);
void query_filter_before(Query *query, time_t time);
void query_filter_after(Query *query, time_t time);
uint32_t query_tell(const Query *query);
uint32_t query_count(const Query *query);
uint32_t query_remaining(const Query *query);

#endif
