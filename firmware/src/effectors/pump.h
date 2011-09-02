/*
 * pump.h - Pump control
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PUMP_H
#define _PUMP_H

#include <inttypes.h>

#include "util/bool.h"

void pump_init(void);
void pump_start(uint8_t plant_idx);
bool pump_is_running(void);
void pump_stop(void);

#endif
