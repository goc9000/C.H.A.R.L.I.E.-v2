/*
 * records.h - Records file functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _RECORDS_H
#define _RECORDS_H

#include <inttypes.h>

#include "time/time.h"
#include "query.h"
#include "dot_config.h"

#define PLANT_FLAGS_NOT_INSTALLED 1

typedef struct {
    uint8_t ilumination; // 0-200 for 0-100%
    uint8_t humidity; // 0-200 for 0-100%
    uint8_t flags;
} PlantStatus;

typedef struct {
    time_t time;
    PlantStatus plants[CFG_MAX_PLANTS];
} Record;

void rec_get_query(Query *query);
void rec_save_record(const Record *record);
void rec_purge(void);

#endif
