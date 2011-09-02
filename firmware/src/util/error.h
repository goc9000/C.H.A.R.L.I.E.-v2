/*
 * error.h - Generic error codes
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _ERROR_H
#define _ERROR_H

#include <inttypes.h>

#define E_OK           0x00
#define E_DEVICE       0xFC
#define E_FORMAT       0xFD
#define E_NOTFOUND     0xFE
#define E_ERROR        0xFF

typedef uint8_t err_t;

#endif
