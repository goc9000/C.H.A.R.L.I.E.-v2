/*
 * filesys.c - File system functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "dev/sd_card.h"
#include "filesys.h"

#define CARD_BLK_MASK 0xFFFFFE00

static struct {
    struct {
        uint8_t buf[512];
        uint32_t address;
        bool dirty:1;
        bool force_reload:1;
    } cache;
} disk;

/**
 * _fsys_flush_cache - Commits the in-memory block to disk
 */
static void _fsys_flush_cache(void)
{
    if (!disk.cache.dirty) {
        return;
    }

    card_set_block_size(512);
    card_write_block(disk.cache.buf, disk.cache.address);

    disk.cache.dirty = FALSE;
}

/**
 * _fsys_cache_touch - Caches the disk block that contains a given address
 * @address: A physical address on disk
 */
static void _fsys_cache_touch(uint32_t address)
{
    if (!disk.cache.force_reload &&
        (disk.cache.address == (address & CARD_BLK_MASK))) {
        return;
    }

    _fsys_flush_cache();

    disk.cache.address = address & CARD_BLK_MASK;
    card_set_block_size(512);
    card_read_block(disk.cache.buf, disk.cache.address);

    disk.cache.force_reload = FALSE;
}

/**
 * _fsys_read_disk - Reads from disk, using the cache
 * @buffer: A buffer for the read data
 * @address: The disk address to read from
 * @length: The number of bytes to read
 */
static void _fsys_read_disk(void *buffer, uint32_t address, uint16_t length)
{
    while (length--) {
        _fsys_cache_touch(address);
        *((uint8_t *)(buffer++)) =
            disk.cache.buf[(address++) & ~CARD_BLK_MASK];
    }
}

/**
 * _fsys_write_disk - Writes to disk, using the cache
 * @buffer: A buffer for the data to be written
 * @address: The disk address to write to
 * @length: The number of bytes to write
 */
static void _fsys_write_disk(const void *buffer, uint32_t address, uint16_t length)
{
    while (length--) {
        _fsys_cache_touch(address);
        disk.cache.buf[(address++) & ~CARD_BLK_MASK] =
            *((uint8_t *)(buffer++));
    }
    
    disk.cache.dirty = TRUE;
}

/**
 * _fsys_get_entry - Gets a file entry from disk
 * @entry_no: The entry number
 * @entry: A structure to hold the entry
 */
static void _fsys_get_entry(uint8_t entry_no, fsys_Entry *entry)
{
    _fsys_read_disk((uint8_t *)entry, sizeof(fsys_Entry) * entry_no,
        sizeof(fsys_Entry));
}

/**
 * _fsys_update_entry - Updates a file entry on disk
 * @entry_no: The entry number
 * @entry: A structure holding the new entry data
 */
static void _fsys_update_entry(uint8_t entry_no, const fsys_Entry *entry)
{
    _fsys_write_disk((uint8_t *)entry, sizeof(fsys_Entry) * entry_no,
        sizeof(fsys_Entry));
    _fsys_flush_cache();
}

/**
 * fsys_check_read_only - Checks whether the SD card is read-only
 */
bool fsys_check_read_only(void)
{
    uint8_t buf[32];
    uint8_t i, j;
    file_t file;
    uint32_t sd_base;

    _delay_ms(1);

    if (file_open_P(&file, PSTR("sys/wrtest.bin"))) {
        return TRUE;
    }
    sd_base = file.sd_base;
    
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 32; i++) {
            buf[i] = 'A' + i + j;
        }

        _fsys_write_disk(buf, sd_base, 32);
        _fsys_flush_cache();

        disk.cache.force_reload = TRUE;

        _fsys_read_disk(buf, sd_base, 32);
        
        for (i = 0; i < 32; i++) {
            if (buf[i] != 'A' + i + j) {
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

err_t _file_open(file_t *file, PGM_P path_a, const char *path_b)
{
    fsys_Entry entry;
    uint8_t entry_no;
    
    for (entry_no = 0; ; entry_no++) {
        _fsys_get_entry(entry_no, &entry);
        if (!entry.filename[0]) {
            return E_NOTFOUND;
        }
        
        if ((path_a && !strcasecmp_P(entry.filename, path_a)) ||
            (path_b && !strcasecmp(entry.filename, path_b))) {
            break;
        }
        
        if (entry_no == 255) {
            return E_NOTFOUND;
        }
    }
        
    file->entry_no = entry_no;
    file->sd_base = entry.sd_base;
    file->cursor = 0;
    file->length = entry.length;
    file->capacity = entry.capacity;

    return E_OK;
}

/**
 * file_open - Open a file
 * @file: The file structure to be initialized
 * @path: The absolute path to the file
 */
err_t file_open(file_t *file, const char* path)
{
    return _file_open(file, NULL, path);
}

/**
 * file_open_P - Open a file using a name stored in program memory
 * @file: The file structure to be initialized
 * @path: The absolute path to the file, stored in program memory
 */
err_t file_open_P(file_t *file, PGM_P path)
{
    return _file_open(file, path, NULL);
}

/**
 * file_append - Prepares appending data to a file
 * @file: An open file object
 */
void file_append(file_t *file)
{
    file_seek(file, file->length);
}

/**
 * file_seek - Sets the current read/write position in a file
 * @file: An open file object
 * @seek_to: The new read/write position
 */
void file_seek(file_t *file, uint32_t seek_to)
{
    if (seek_to > file->length) {
        seek_to = file->length;
    }
    file->cursor = seek_to;
}

/**
 * file_read - Reads from a file
 * @file: An open file object
 * @buffer: A buffer to hold the read data
 * @length: The maximum number of bytes to read
 * 
 * Returns the number of bytes actually read.
 */
uint16_t file_read(file_t *file, void *buffer, uint16_t length)
{
    if (file->cursor > file->length) {
        length = 0;
    } else if (file->length-file->cursor < length) {
        length = file->length-file->cursor;
    }
    
    _fsys_read_disk(buffer, file->sd_base + file->cursor, length);
    file->cursor += length;

    return length;
}

/**
 * file_write - Writes to a file
 * @file: An open file object
 * @buffer: A buffer that holds the data to be written
 * @length: The number of bytes to write
 * 
 * Returns the number of bytes actually written.
 */
uint16_t file_write(file_t *file, const void *buffer, uint16_t length)
{
    if (file->cursor > file->capacity) {
        length = 0;
    } else if (file->capacity-file->cursor < length) {
        length = file->capacity-file->cursor;
    }
    
    _fsys_write_disk(buffer, file->sd_base + file->cursor, length);
    file->cursor += length;
    if (file->cursor > file->length) {
        file->length = file->cursor;
    }

    return length;
}

/**
 * file_truncate - Deletes all data in a file
 * @file: An open file object
 */
void file_truncate(file_t *file)
{
    file->length = 0;
    file->cursor = 0;
}

/**
 * file_flush - Flushes pending data in a file to disk
 * @file: An open file object
 */
void file_flush(file_t *file)
{
    fsys_Entry entry;
    
    _fsys_get_entry(file->entry_no, &entry);
    entry.length = file->length;
    _fsys_update_entry(file->entry_no, &entry);
}

/**
 * file_size - Gets the size of an open file
 * @file: An open file object
 */
uint32_t file_size(const file_t *file)
{
    return file->length;
}

/**
 * file_eof - Checks for end-of-file
 * @file: An open file object
 */
bool file_eof(const file_t *file)
{
    return (file->cursor == file->length);
}

/**
 * fsys_init - Initializes the file system interface
 * 
 * Note: the SD card must have been initialized beforehand
 */
err_t fsys_init(void)
{
    disk.cache.force_reload = TRUE;
    disk.cache.dirty = FALSE;

    return E_OK;
}
