/*
 * charlie.h - Main program
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _CHARLIE_H
#define _CHARLIE_H

#include "util/bool.h"

#define CHARLIE_STAT_OK            0
#define CHARLIE_STAT_RTC_ERROR     1
#define CHARLIE_STAT_CARD_ERROR    2
#define CHARLIE_STAT_FSYS_ERROR    3
#define CHARLIE_STAT_FSYS_READONLY 4
#define CHARLIE_STAT_NET_ERROR     5

void shutdown(bool restart);

#endif
