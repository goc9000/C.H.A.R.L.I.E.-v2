/*
 * time.h - Time functions
 * 
 * Note: time_t <-> tm_struct conversion functions courtesy of:
 * http://www.maxim-ic.com/app-notes/index.mvp/id/517
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _TIME_H
#define _TIME_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#include "util/bool.h"

#define TIME_SECONDS_PER_DAY 86400

typedef uint32_t time_t;

typedef struct {
   uint8_t tm_sec;
   uint8_t tm_min;
   uint8_t tm_hour;
   uint8_t tm_mday;
   uint8_t tm_mon;
   uint8_t tm_year;
   uint8_t tm_wday;
   uint16_t tm_yday;
   uint8_t tm_isdst;
} tm;

void time_init(void);
void time_set_raw(time_t new_time);
time_t time_get_raw(void);
uint8_t time_get_centis(void);
uint32_t time_get_uptime(void);
PGM_P time_get_short_month_name(uint8_t month);
void time_convert_from_raw(tm *time, time_t timestamp);
time_t time_convert_to_raw(const tm *time);
bool time_same_date(time_t time_a, time_t time_b);
int8_t time_compare_monotonic(uint32_t time_a, uint32_t time_b);

#endif
