/*
 * debug.c - Debug functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <util/delay.h>
#include <ctype.h>
#include <stdlib.h>

#include "interf/rs232.h"
#include "net/dev/enc28j60.h"
#include "net/proto/ip.h"
#include "net/proto/eth.h"
#include "net/packet_buf.h"
#include "effectors/pump.h"
#include "util/hex.h"
#include "util/misc.h"
#include "debug.h"

#if CFG_DEBUG_ENABLED==1
void _debug_printf(PGM_P format, ...)
{
    va_list args;
    char buf[24];
    uint8_t *ptr;
    uint16_t length;
    uint8_t alt, byt;
    char c;
    PacketBuf *pkt;
    
    va_start(args, format);
    
    for ( ; (c = pgm_read_byte(format)); format++) {
        if (c == '%') {
            format++;
            c = pgm_read_byte(format);
            switch (c) {
                case 'd':
                    rs232_puts(itoa10(va_arg(args, int), buf));
                    continue;
                case 'l':
                    rs232_puts(ltoa10(va_arg(args, long), buf));
                    continue;
                case 's':
                    rs232_puts(va_arg(args, const char*));
                    continue;
                case 'S': // Program memory string
                    rs232_puts_P(va_arg(args, PGM_P));
                    continue;
                case 'X': // Hexdump (must be followed by int specifying length)
                    ptr = (uint8_t *)va_arg(args, void*);
                    length = va_arg(args, int);
                    alt = 0;
                    
                    while (length--) {
                        byt = *(ptr++);
                        alt++;
                        
                        hex_format(buf, 2, byt);
                        rs232_puts(buf);
                            
                        if (length) {
                            if (alt == 16) {
                                rs232_putc('\n');
                                alt = 0;
                            } else {
                                rs232_putc(' ');
                            }
                        }
                    }
                    continue;
                case 'i': // IP address
                    ip_format(buf, va_arg(args, ip_addr_t*));
                    rs232_puts(buf);
                    continue;
                case 'm': // MAC address
                    eth_format_mac(buf, va_arg(args, mac_addr_t*));
                    rs232_puts(buf);
                    continue;
                case 'k': // Packet buffer
                    pkt = va_arg(args, PacketBuf*);
                    rs232_puts_P(PSTR("<@"));
                    hex_format(buf, 4, pkt->start);
                    rs232_puts(buf);
                    rs232_puts_P(PSTR(","));
                    rs232_puts(itoa10(pkt->length, buf));
                    rs232_puts_P(PSTR("b:\""));
                    enc28j60_read_mem_at(buf, pkt->start, 8);
                    buf[8] = 0;
                    rs232_puts(buf);
                    rs232_puts_P(PSTR("\">"));
                    continue;
                case '%':
                    rs232_putc('%');
                    continue;
                default:
                    rs232_putc('%');
                    break;
            }
        }
        rs232_putc(c);
    }
    
    va_end(args);
}
#endif

/**
 * debug_init_ports - Initializes the ports for the debug LED and key
 */
void debug_init_ports(void)
{
    CFG_DEBUG_LED_DDR |= _BV(CFG_DEBUG_LED_BIT);
    CFG_DEBUG_KEY_DDR &= ~_BV(CFG_DEBUG_KEY_BIT);
}

/**
 * debug_failure_mode - Halt indicating severe failure
 * @err_code: The error code
 * 
 * Puts the device in a state indicating a severe intialization failure.
 * The debug LED will exhibit periodic bursts, the number of pulses in
 * each burst corresponding to the error code (1-5 max).
 */
void debug_failure_mode(uint8_t err_code)
{
    uint8_t i;
    
    debug_init_ports();

    while (TRUE) {
        for (i = 0; i < err_code; i++) {
            debug_led_on();
            _delay_ms(100);
            debug_led_off();
            _delay_ms(300);
        }
        
        _delay_ms(1000);
    }
}

/**
 * debug_failure_mode2 - Halt indicating severe failure, extended
 * @param err_code The error code
 * 
 * Similar to debug_failure_mode(), but uses a different method of
 * displaying the error code that makes it feasible to use any error
 * code from 0 to 255; that is, the debug LED will blink once or twice
 * for every bit in the error code, depending on whether it is 0 or 1,
 * respectively.
 */
void debug_failure_mode2(uint8_t err_code)
{
    uint8_t i;
    
    debug_init_ports();

    while (TRUE) {
        for (i = 0; i < 8; i++) {
            debug_led_on();
            _delay_ms(50);
            debug_led_off();
            if (err_code & _BV(7-i)) {
                _delay_ms(50);
                debug_led_on();
                _delay_ms(50);
                debug_led_off();
            }
            _delay_ms(350);
        }
        
        _delay_ms(1500);
    }
}

/**
 * debug_pump_test - Enter pump test mode
 * 
 * Puts the device in a mode suitable for testing the pump subsystem.
 * The pump attached to plant #1 will be activated for as long as the
 * debug key is depressed.
 */
void debug_pump_test(void)
{
    uint8_t debounce = 0;
    bool on = FALSE;
    
    debug_init_ports();

    debug_led_off();
    while (TRUE) {
        debounce += (debug_key_down() && (debounce < 255));
        debounce -= (debug_key_up() && (debounce > 0));

        if (!on && (debounce > 127)) {
            debug_led_on();
            pump_start(0);
            on = TRUE;
        } else if (on && (debounce < 128)) {
            debug_led_off();
            pump_stop();
            on = FALSE;
        }

        _delay_ms(1);
    }
}
