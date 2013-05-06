/*
 * time.c - Time functions
 * 
 * Note: time_t <-> tm_struct conversion functions courtesy of:
 * http://www.maxim-ic.com/app-notes/index.mvp/id/517
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>

#include "time.h"

volatile struct {
    volatile time_t counter; // Main counter: # of seconds since 1970
    volatile uint8_t centis; // Secondary counter: centiseconds part
    volatile uint32_t uptime; // Uptime counter (centiseconds)
    volatile bool lock; // Lock for use during updates
} time;

/**
 * Timer interrupt service routine.
 */
ISR(TIMER1_COMPA_vect)
{
    time.uptime++;
    if (!time.lock) {
        time.centis++;
        if (time.centis == 100) {
            time.centis = 0;
            time.counter++;
        }
    }
}

/**
 * time_set_raw - Sets the current time
 * @new_time: The new time, as a Unix timestamp
 */
void time_set_raw(time_t new_time)
{
    time.lock = 1;
    time.counter = new_time;
    time.lock = 0;
}

/**
 * time_get_raw - Get current time as a Unix timestamp
 */
time_t time_get_raw(void)
{
    time_t time_val;
    do {
        time_val = time.counter;
    } while (time.counter != time_val);

    return time_val;
}

/**
 * time_get_centis - Get the centiseconds in the current time
 */
uint8_t time_get_centis(void)
{
    return time.centis;
}

/**
 * time_get_uptime - Gets the uptime, in centiseconds
 */
uint32_t time_get_uptime(void)
{
    uint32_t centis;
    do {
        centis = time.uptime;
    } while (centis != time.uptime);

    return centis;
}

/**
 * time_get_days_to_month - Gets a month's offset, in days, relative
 *                          to the start of the year
 * @month: The month (0 is January)
 * @year: The year
 */
static uint16_t time_get_days_to_month(uint8_t month, uint16_t year)
{
    static uint16_t DAYS_TO_MONTH[13] = {
       0,31,59,90,120,151,181,212,243,273,304,334,365
    };
    uint16_t result = DAYS_TO_MONTH[month];

    if ((month > 2) && !(year & 3)) {
        result++;
    }

    return result;
}

/**
 * time_get_short_month_name - Gets a string for the short name of a month
 * @month: The month (0 is January)
 */
PGM_P time_get_short_month_name(uint8_t month)
{
    return PSTR("Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Oct\0Sep\0Nov\0Dec\0???\0") +
        (((month>12) ? 12 : month) << 2);
}

/**
 * time_convert_from_raw - Converts a Unix timestamp to broken-down time
 * @time: A structure to receive the broken-down time
 * @timestamp: The timestamp to convert
 */
void time_convert_from_raw(tm *time, time_t timestamp)
{   
    uint32_t total_minutes = timestamp / 60;
    uint32_t total_hours  = total_minutes / 60;
    uint32_t total_days = total_hours / 24;
    
    uint32_t days_since_1968 = total_days + 365 + 366;
    uint32_t days_since_leap = days_since_1968 % ((4*365)+1);
    uint32_t leap_year_periods = (days_since_1968 / ((4*365)+1))
        + (days_since_leap >= (31 + 29));
    uint32_t years = (days_since_1968 - leap_year_periods) / 365;
    uint32_t day_in_year = days_since_1968 - (years * 365)
        - leap_year_periods
        + ((days_since_leap <= 365) && (days_since_leap >= 60));
    uint32_t month = 13;
    uint32_t days_to_month = 366;
    
    while (day_in_year < days_to_month) {
        month--;
        days_to_month = time_get_days_to_month(month, years + 68);
    }

    time->tm_sec = timestamp % 60;
    time->tm_min = total_minutes % 60;
    time->tm_hour = total_hours % 24;
    time->tm_yday = day_in_year;
    time->tm_mday = day_in_year - days_to_month + 1;
    time->tm_wday = (total_days  + 4) % 7;
    time->tm_mon  = month;
    time->tm_year = years + 68;
}

/**
 * time_convert_to_raw - Converts broken-down time to a Unix timestamp
 * @time: The broken-down time
 */
time_t time_convert_to_raw(const tm *time)
{
    uint32_t days = 365 * (time->tm_year - 70) +
        time_get_days_to_month(time->tm_mon, time->tm_year) +
        (time->tm_mday - 1) +
        (time->tm_year - 69) / 4;
    
    return time->tm_sec + 60 * time->tm_min + 3600 * (time->tm_hour + 24 * days);
}

/**
 * time_same_date - Checks whether two timestamps occur on the same date
 * @time_a: A timestamp
 * @time_b: Another timestamp
 */
bool time_same_date(time_t time_a, time_t time_b)
{
    return ((time_a/86400) == (time_b/86400));
}

/**
 * time_compare_monotonic - Compares two monotonic time (uptime) values
 * @time_a: A timestamp
 * @time_b: Another timestamp
 * 
 * The function takes potential wrapping into account. The result is
 * -1 for A<B, 0 for A==B and 1 for A>B.
 */
int8_t time_compare_monotonic(uint32_t time_a, uint32_t time_b)
{
    if (time_a == time_b) {
        return 0;
    }
    
    if (time_a > time_b) {
        return ((time_a - time_b) >> 31) ? -1 : 1;
    }
    
    return ((time_b - time_a) >> 31) ? 1 : -1;
}

static void _itoa2(char *buf, uint8_t number)
{
    buf[0] = '0' + (number / 10);
    buf[1] = '0' + (number % 10);
}

/**
 * time_format_rfc3339 - Renders a timestamp in RFC3339 format.
 * @buffer: A buffer to store the representation. Must be at least 20
 *          characters wide.
 * @datetime: The time to format
 */
void time_format_rfc3339(char *buffer, time_t datetime)
{
    tm time;
    
    strcpy_P(buffer, PSTR("0000-00-00T00:00:00"));
    
    if (!datetime) {
        buffer[10] = 0;
        return;
    }
    
    time_convert_from_raw(&time, datetime);
    
    _itoa2(buffer+ 0, 19 + (time.tm_year / 100));
    _itoa2(buffer+ 2, time.tm_year % 100);
    _itoa2(buffer+ 5, time.tm_mon + 1);
    _itoa2(buffer+ 8, time.tm_mday);
    _itoa2(buffer+11, time.tm_hour);
    _itoa2(buffer+14, time.tm_min);
    _itoa2(buffer+17, time.tm_sec);
}

/**
 * time_init - Initializes timekeeping
 * 
 * Note that interrupts are not automatically enabled.
 */
void time_init(void)
{
    time.counter = 0;
    time.centis = 0;
    time.uptime = 0;
    time.lock = FALSE;
    
    // 100Hz clock interrupt
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS11);
    OCR1A = 0x4E20;
    TIMSK = _BV(OCIE1A);
}
