/*
 * debug.h - Debug functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#include <inttypes.h>
#include <stdarg.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "dot_config.h"
#include "util/bool.h"

#define debug_led_on()   CFG_DEBUG_LED_PORT |= _BV(CFG_DEBUG_LED_BIT)
#define debug_led_off()  CFG_DEBUG_LED_PORT &= ~_BV(CFG_DEBUG_LED_BIT)
#define debug_key_down() (!(CFG_DEBUG_KEY_PORT & _BV(CFG_DEBUG_KEY_BIT)))
#define debug_key_up()   (CFG_DEBUG_KEY_PORT & _BV(CFG_DEBUG_KEY_BIT))

#if CFG_DEBUG_ENABLED==1
#define debug_printf(format, ...) _debug_printf(PSTR(format), ## __VA_ARGS__)
void _debug_printf(PGM_P format, ...);
#else
#define debug_printf(format, ...)
#endif

void debug_init_ports(void);
void debug_failure_mode(uint8_t err_code);
void debug_failure_mode2(uint8_t err_code);
void debug_rootfs_upload_mode(void);
void debug_pump_test(void);

#endif
