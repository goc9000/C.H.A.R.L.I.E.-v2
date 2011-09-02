/*
 * plants.h - Plant watering & monitoring
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PLANTS_H
#define _PLANTS_H

#include <inttypes.h>

#include "model/records.h"
#include "util/bool.h"

void plants_get_latest_record(Record *record);
bool plants_busy(void);
void plants_init(void);

#endif
