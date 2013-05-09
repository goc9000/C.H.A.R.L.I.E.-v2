/*
 * records.c - Records file functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */
 
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>

#include "storage/filesys.h"
#include "records.h"

/**
 * rec_get_query - Create a query on the records file
 * @query: The query structure that will be initialized
 */
void rec_get_query(Query *query)
{
    query_make_P(query, PSTR(CFG_RECORDS_FILE), sizeof(Record));
}

/**
 * rec_save_record - Add a record to the records file
 * @record: The record to be added
 */
void rec_save_record(const Record *record)
{
    file_t file;
    file_open_P(&file, PSTR(CFG_RECORDS_FILE));
    file_append(&file);
    file_write(&file, (void *)record, sizeof(Record));
    file_flush(&file);
}

/**
 * rec_purge - Deletes all records
 */
void rec_purge(void)
{
    file_t file;
    file_open_P(&file, PSTR(CFG_RECORDS_FILE));
    file_truncate(&file);
    file_flush(&file);
}
