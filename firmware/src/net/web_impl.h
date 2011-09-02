/*
 * web_impl.h - HTTP/Web layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _WEB_IMPL_H
#define _WEB_IMPL_H

#include <inttypes.h>

#include "packet_buf.h"
#include "util/bool.h"

void web_new_request(void);
bool web_receive_request(PacketBuf *data);
bool web_get_answer(PacketBuf *data);

#endif
