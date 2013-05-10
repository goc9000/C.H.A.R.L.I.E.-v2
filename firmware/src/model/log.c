/*
 * log.c - Log file functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>

#include "dot_config.h"
#include "storage/filesys.h"
#include "util/misc.h"
#include "log.h"

/**
 * log_get_query - Create a query on the log file
 * @query: The query structure that will be initialized
 */
void log_get_query(Query *query)
{
    query_make_P(query, PSTR(CFG_LOG_FILE), sizeof(LogEntry));
}

/**
 * log_make_entry - Makes an entry in the log file
 * @code: The event code (a LOG_EVENT_* constant)
 * @data: Event-specific parameters
 */
void log_make_entry(uint8_t code, uint32_t data)
{
    file_t file;
    LogEntry event;
    
    event.time = time_get_raw();
    event.code = code;
    event.data = data;

    file_open_P(&file, PSTR(CFG_LOG_FILE));
    file_append(&file);
    file_write(&file, &event, sizeof(LogEntry));
    file_flush(&file);
}

/**
 * log_purge - Deletes all log entries
 */
void log_purge(void)
{
    file_t file;
    file_open_P(&file, PSTR(CFG_LOG_FILE));
    file_truncate(&file);
    file_flush(&file);
}

/**
 * log_format_message - Formats a log entry message as a string
 * @buf: A buffer where the message will be stored
 * @entry: The log entry
 */
void log_format_message(char *buf, const LogEntry *entry)
{
    PGM_P format;
    
    *buf = 0;

    format = PSTR(CFG_LOG_TEXT_UNKNOWN);
    switch (entry->code) {
        case LOG_EVENT_ACTIVATED:
            format = PSTR(CFG_LOG_TEXT_ACTIVATED);
            break;
        case LOG_EVENT_WATERING_PLANT:
            format = PSTR(CFG_LOG_TEXT_WATERING);
            break;
        case LOG_EVENT_DONE_WATERING:
            format = PSTR(CFG_LOG_TEXT_WATERED);
            break;
        case LOG_EVENT_SHUTTING_DOWN:
            format = PSTR(CFG_LOG_TEXT_SHUTDOWN);
            break;
        case LOG_EVENT_RESTARTING:
            format = PSTR(CFG_LOG_TEXT_RESTARTING);
            break;
        case LOG_EVENT_TIME_LOST:
            format = PSTR(CFG_LOG_TEXT_TIME_LOST);
            break;
    }
    strcpy_P(buf, format);
    
    switch (entry->code) {
        case LOG_EVENT_WATERING_PLANT:
        case LOG_EVENT_DONE_WATERING:
            ltoa10(entry->data+1, buf+strlen(buf));
            break;
    }
}
