/*
 * rtc_ds1307.h - DS1307 real-time clock functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _RTC_DS1307_H
#define _RTC_DS1307_H

#include "time/time.h"
#include "util/bool.h"
#include "util/error.h"

#define DS1307_REG_SECONDS  0x00
#define DS1307_REG_MINUTES  0x01
#define DS1307_REG_HOURS    0x02
#define DS1307_REG_DOW      0x03
#define DS1307_REG_DATE     0x04
#define DS1307_REG_MONTH    0x05
#define DS1307_REG_YEAR     0x06
#define DS1307_REG_CONTROL  0x07

#define DS1307_CH_BIT       7
#define DS1307_12HR_BIT     6
#define DS1307_PM_BIT       5

err_t rtc_init(void);
bool rtc_time_lost(void);
time_t rtc_read(void);
void rtc_set(time_t time);

#endif
