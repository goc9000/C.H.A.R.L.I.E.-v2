/*
 * filesys.h - File system functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _FILESYS_H
#define _FILESYS_H

#include <avr/pgmspace.h>

#include "util/bool.h"
#include "util/error.h"

typedef struct {
    uint8_t entry_no;
    uint32_t sd_base;
    uint32_t cursor;
    uint32_t length;
    uint32_t capacity;
} file_t;

typedef struct {
    char filename[52];
    uint32_t sd_base;
    uint32_t length;
    uint32_t capacity;
} fsys_Entry;

err_t fsys_init(void);
bool fsys_check_read_only(void);

err_t file_open(file_t *file, const char *path);
err_t file_open_P(file_t *file, PGM_P path);
void file_append(file_t *file);
void file_seek(file_t *file, uint32_t seek_to);
uint16_t file_read(file_t *file, void *buffer, uint16_t length);
uint16_t file_write(file_t *file, const void *buffer, uint16_t length);
void file_truncate(file_t *file);
void file_flush(file_t *file);
uint32_t file_size(const file_t *file);
bool file_eof(const file_t *file);

#endif
