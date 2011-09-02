/*
 * rs232.h - RS232 communication functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _RS232_H
#define _RS232_H

#include <avr/io.h>
#include <avr/pgmspace.h>

void rs232_init(void);
void rs232_putc(char c);
char rs232_getc(void);
void rs232_puts(const char *str);
void rs232_puts_P(PGM_P str);

#endif
