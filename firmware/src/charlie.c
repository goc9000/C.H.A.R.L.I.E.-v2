/*
 * charlie.c - Main program
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "debug/debug.h"
#include "effectors/pump.h"
#include "interf/rs232.h"
#include "interf/spi.h"
#include "interf/i2c.h"
#include "model/log.h"
#include "storage/dev/sd_card.h"
#include "storage/filesys.h"
#include "core/config.h"
#include "core/sched.h"
#include "core/plants.h"
#include "time/time.h"
#include "time/dev/rtc_ds1307.h"
#include "sensors/adc.h"
#include "sensors/sensors.h"
#include "net/dev/enc28j60.h"
#include "net/net_core.h"
#include "charlie.h"

typedef void (*funcptr)(void);

static struct {
    bool shutdown:1;
    bool restart:1;
} charlie;

// Catch spurious interrupts
ISR(BADISR_vect)
{
}

/**
 * init_atmega - Initializes ATMEGA resources (registers, interrupts etc.)
 */
static void init_atmega(void)
{
}

/**
 * init_ports - Initializes ports to a passive state
 */
static void init_ports(void)
{
    DDRA = DDRB = DDRC = DDRD = 0x00;
    PORTA = PORTB = PORTC = PORTD = 0xFF;
    DDRC |= _BV(CFG_PUMP_MUX_ENABLE_BIT);

    // Wait for voltage levels to stabilize
    _delay_ms(100); 
}

/**
 * initialize - Performs all initialization procedures
 *
 * Returns a CHARLIE_STAT_* constant that indicates the succes or
 * failure of the initialization.
 */
static uint8_t initialize(void)
{
    charlie.shutdown = FALSE;
    
    init_atmega();
    init_ports();
    debug_init_ports();
    
    rs232_init();
    spi_init_master();
    i2c_init();
    
    if (rtc_init()) {
        return CHARLIE_STAT_RTC_ERROR;
    }
    time_init();
    time_set_raw(rtc_read());
    
    sched_init();
    
    if (card_init()) {
        return CHARLIE_STAT_CARD_ERROR;
    }
    if (fsys_init()) {
        return CHARLIE_STAT_FSYS_ERROR;
    }
    if (fsys_check_read_only()) {
        return CHARLIE_STAT_FSYS_READONLY;
    }   
    cfg_load();
    if (enc28j60_init(&cfg.mac_addr)) {
        return CHARLIE_STAT_NET_ERROR;
    }
    net_init();
    
    adc_init();
    sensors_init();
    pump_init();
    plants_init();
    
    sei();

    return CHARLIE_STAT_OK;
}

/**
 * shutdown_state - Puts the device into a safe state for shutdown
 */
static void shutdown_state(void)
{
    cli();

    while (TRUE) {
        debug_led_on();
        _delay_ms(50);
        debug_led_off();
        _delay_ms(1000);
    }
}

/**
 * shutdown - Requests device restart/shutdown
 * @restart: TRUE for a restart command, FALSE for shutdown
 */
void shutdown(bool restart)
{
    charlie.shutdown = TRUE;
    charlie.restart = restart;
}

/**
 * main - Main program
 */
int main(void)
{
    // TODO: watchdog
    uint8_t stat = initialize();
    if (stat != CHARLIE_STAT_OK) {
        debug_failure_mode(stat);
    }

    log_make_entry(LOG_EVENT_ACTIVATED, 0);
    if (rtc_time_lost()) {
        log_make_entry(LOG_EVENT_TIME_LOST, 0);
    }

    while (TRUE) {
        if (debug_key_down()) {
            shutdown(FALSE);
        }
        
        sched_run();
        _delay_ms(1);
        
        if (charlie.shutdown && !plants_busy() && !net_busy()) {
            break;
        }
    }

    log_make_entry(charlie.restart ?
        LOG_EVENT_RESTARTING : LOG_EVENT_SHUTTING_DOWN, 0);

    if (charlie.restart) {
        ((funcptr)0)();
    }

    shutdown_state();
    
    return 0;
}
