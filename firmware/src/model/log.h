/*
 * log.h - Log file functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _LOG_H
#define _LOG_H

#include <inttypes.h>

#include "time/time.h"
#include "query.h"

#define LOG_EVENT_MASK    0x3F
#define LOG_EVENT_WARNING 0x40
#define LOG_EVENT_ERROR   0x80

#define LOG_EVENT_ACTIVATED      1
#define LOG_EVENT_WATERING_PLANT 2
#define LOG_EVENT_DONE_WATERING  3

#define LOG_EVENT_SHUTTING_DOWN  LOG_EVENT_WARNING+1
#define LOG_EVENT_RESTARTING     LOG_EVENT_WARNING+2

#define LOG_EVENT_TIME_LOST      LOG_EVENT_ERROR+1

typedef struct {
    time_t time;
    uint8_t code;
    uint32_t data;
} LogEntry;

void log_get_query(Query *query);
void log_make_entry(uint8_t code, uint32_t data);
void log_purge(void);
void log_format_message(char *buf, const LogEntry *entry);

#endif
