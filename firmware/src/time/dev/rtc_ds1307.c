/*
 * rtc_ds1307.c - DS1307 real-time clock functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/io.h>
#include <string.h>

#include "debug/debug.h"
#include "interf/i2c.h"
#include "dot_config.h"
#include "rtc_ds1307.h"

static bool rtc_time_is_lost;

static uint8_t _bcd2dec(uint8_t bcd)
{
    return 10*((bcd >> 4) & 0x0F) + (bcd & 0x0F);
}

static uint8_t _dec2bcd(uint8_t dec)
{
    return ((dec / 10) << 4) + (dec % 10);
}

static bool _rtc_read_seq(uint8_t reg, uint8_t *buf, uint8_t count)
{
    bool ok = FALSE;

    if (i2c_start_write(CFG_RTC_I2C_ADDR) && i2c_write(reg)) {
        ok = i2c_start_read(CFG_RTC_I2C_ADDR);
        while (count && ok) {
            count--;
            if (count) {
                *(buf++) = i2c_read();
            } else {
                *(buf++) = i2c_read_last();
            }
        }
    }
    
    i2c_stop();
    
    return ok;
}

static bool _rtc_write_seq(uint8_t reg, const uint8_t *buf, uint8_t count)
{
    bool ok = FALSE;

    if (i2c_start_write(CFG_RTC_I2C_ADDR) && i2c_write(reg)) {
        ok = TRUE;
        while (count && ok) {
            ok = i2c_write(*(buf++));
            count--;
        }
    }
    
    i2c_stop();
    
    return ok;
}

static uint8_t _rtc_read(uint8_t reg) __attribute__((unused));

static uint8_t _rtc_read(uint8_t reg)
{
    uint8_t data;
    
    _rtc_read_seq(reg, &data, 1);
    
    return data;
}

static bool _rtc_write(uint8_t reg, uint8_t data)
{
    return _rtc_write_seq(reg, &data, 1);
}

/**
 * rtc_read - Read the time in the RTC in Unix timestamp format
 */
time_t rtc_read(void)
{
    tm time;
    uint8_t regs[7];
    uint8_t hour;
    
    // Note: the DS1307 manual states that this is safe (the value
    // is cached every I2C START, so it will never change during a read)
    _rtc_read_seq(DS1307_REG_SECONDS, regs, 7);
    
    rtc_time_is_lost = ((regs[0] & _BV(DS1307_CH_BIT)) != 0);
    regs[0] &= ~_BV(DS1307_CH_BIT);
    
    time.tm_sec = _bcd2dec(regs[DS1307_REG_SECONDS]);
    time.tm_min = _bcd2dec(regs[DS1307_REG_MINUTES]);
    if (regs[DS1307_REG_HOURS] & _BV(DS1307_12HR_BIT)) {
        hour = _bcd2dec(regs[DS1307_REG_HOURS] & 0x1F);
        if (hour == 12) {
            hour = 0;
        }
        if (regs[DS1307_REG_HOURS] & _BV(DS1307_PM_BIT)) {
            hour += 12;
        }
        time.tm_hour = hour;
    } else {
        time.tm_hour = _bcd2dec(regs[DS1307_REG_HOURS] & 0x3F);
    }
    time.tm_mday = _bcd2dec(regs[DS1307_REG_DATE]);
    time.tm_mon = _bcd2dec(regs[DS1307_REG_MONTH])-1;
    time.tm_year = _bcd2dec(regs[DS1307_REG_YEAR]);
    if (time.tm_year < 70) {
        time.tm_year += 100;
    }
    
    return time_convert_to_raw(&time);
}

/**
 * rtc_time_set - Sets the time in the RTC
 * @time: The time to set, in Unix timestamp format
 */
void rtc_set(time_t time)
{
    tm ti;
    uint8_t regs[7];
    
    time_convert_from_raw(&ti, time);
    regs[DS1307_REG_SECONDS] = _dec2bcd(ti.tm_sec);
    regs[DS1307_REG_MINUTES] = _dec2bcd(ti.tm_min);
    regs[DS1307_REG_HOURS]   = _dec2bcd(ti.tm_hour);
    regs[DS1307_REG_DOW]     = _dec2bcd(ti.tm_wday) + 1;
    regs[DS1307_REG_DATE]    = _dec2bcd(ti.tm_mday);
    regs[DS1307_REG_MONTH]   = _dec2bcd(ti.tm_mon) + 1;
    regs[DS1307_REG_YEAR]    = _dec2bcd(ti.tm_year % 100);
    
    _rtc_write_seq(DS1307_REG_SECONDS, regs, 7);
    
    rtc_time_is_lost = FALSE;
}

/**
 * rtc_time_lost - Checks whether the RTC has lost its time settings
 */
bool rtc_time_lost(void)
{
    return rtc_time_is_lost;
}

/**
 * rtc_init - Initializes the RTC
 */
err_t rtc_init(void)
{
    uint8_t stat;
    
    if (!_rtc_read_seq(DS1307_REG_SECONDS, &stat, 1)) {
        return E_DEVICE;
    }
    
    rtc_time_is_lost = ((stat & _BV(DS1307_CH_BIT)) != 0);
    
    _rtc_write(DS1307_REG_CONTROL, 0);
        
    return E_OK;
}
