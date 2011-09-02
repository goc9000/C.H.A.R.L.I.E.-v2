/*
 * php_impl.h - Dynamic pages layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PHP_IMPL_H
#define _PHP_IMPL_H

#include <inttypes.h>

#include "packet_buf.h"
#include "util/bool.h"

bool php_start(const char* url_path, PacketBuf *params);
uint16_t php_read(void *buffer, uint16_t length);
bool php_eof(void);
uint8_t php_get_content_type(void);
bool php_is_download(void);
void php_get_download_filename(char *buf);

#endif
