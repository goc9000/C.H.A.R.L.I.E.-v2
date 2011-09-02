/*
 * rs232.c - RS232 communication functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/interrupt.h>

#include "dot_config.h"
#include "rs232.h"

#define BAUD      CFG_RS232_BAUD
#define BAUD_TOL  3
#include <util/setbaud.h>

#if CFG_RS232_STOP_BITS == 2
#define UCSRC_STOP_BITS_VALUE _BV(USBS)
#else
#define UCSRC_STOP_BITS_VALUE 0
#endif

#if CFG_RS232_PARITY == 1
#define UCSRC_PARITY_VALUE _BV(UPM1) | _BV(UPM0)
#elif CFG_RS232_PARITY == 2
#define UCSRC_PARITY_VALUE _BV(UPM1)
#else
#define UCSRC_PARITY_VALUE 0
#endif

/**
 * rs232_putc - Writes a character to the RS232 interface
 * @c The character to be written
 */
void rs232_putc(char c)
{
    if (c == '\r') {
        return;
    }
    
    if (c == '\n') {
        loop_until_bit_is_set(UCSRA, UDRE);
        UDR = '\r';
    }
    
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
}

/**
 * rs232_getc - Reads a character off the RS232 interface
 * 
 * The function blocks until a character is received. The character
 * will also be echoed back.
 */
char rs232_getc(void)
{
    char c;
    
    do {
        loop_until_bit_is_set(UCSRA, RXC);
        c = UDR;
    } while (c != '\r');
    rs232_putc(c);

    return c;
}

/**
 * rs232_puts - Writes a string to the RS232 interface
 * @str: The string to be written
 */
void rs232_puts(const char *str)
{
    char c;

    while ((c = *str)) {
        rs232_putc(c);
        str++;
    }
}

/**
 * rs232_puts_P - Like rs232_puts, for a string stored in program memory
 * @str: The string to be written, stored in program memory
 */
void rs232_puts_P(PGM_P str)
{
    prog_char c;

    while ((c = pgm_read_byte(str))) {
        rs232_putc(c);
        str++;
    }
}

/**
 * rs232_init - Initializes the RS232 interface
 */
void rs232_init(void)
{
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;

    UCSRA =  (USE_2X) << U2X;
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) |
        UCSRC_STOP_BITS_VALUE | UCSRC_PARITY_VALUE;
    UCSRB = _BV(RXEN) | _BV(TXEN);
}
