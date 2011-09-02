/*
 * pump.c - Pump control
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */
 
/* 
 * Note that only one pump may be active at one time.
 */
 
#include <avr/io.h>

#include "dot_config.h"

#include "pump.h"

#define MUX_MASK (_BV(CFG_PUMP_MUX_ADDR_LSB_BIT + CFG_PUMP_MUX_WIDTH) -\
    _BV(CFG_PUMP_MUX_ADDR_LSB_BIT))

struct {
    bool active;
} pump;

/**
 * pump_start - Activates the pump for a given plant
 * @plant_idx: The index of the plant (0-based)
 */
void pump_start(uint8_t plant_idx)
{
    CFG_PUMP_MUX_PORT &= ~MUX_MASK;
    CFG_PUMP_MUX_PORT |= (plant_idx << CFG_PUMP_MUX_ADDR_LSB_BIT);
    CFG_PUMP_MUX_PORT &= ~_BV(CFG_PUMP_MUX_ENABLE_BIT);
    pump.active = TRUE;
}

/**
 * pump_is_running - Checks whether a pump is currently running
 */
bool pump_is_running(void)
{
    return pump.active;
}

/**
 * pump_stop - Stops any currently running pump
 */
void pump_stop(void)
{
    CFG_PUMP_MUX_PORT |= _BV(CFG_PUMP_MUX_ENABLE_BIT);
    pump.active = FALSE;
}

/**
 * pump_init - Initializes the pump subsystem
 */
void pump_init(void)
{
    CFG_PUMP_MUX_DDR |= _BV(CFG_PUMP_MUX_ENABLE_BIT) | MUX_MASK;
    CFG_PUMP_MUX_PORT &= ~MUX_MASK;
    CFG_PUMP_MUX_PORT |= _BV(CFG_PUMP_MUX_ENABLE_BIT);
    pump.active = FALSE;
}
