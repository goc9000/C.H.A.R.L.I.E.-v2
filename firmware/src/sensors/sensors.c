/*
 * sensors.c - Sensors subsystem functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "adc.h"
#include "sensors.h"

#define MUX_MASK (_BV(CFG_SENSOR_MUX_ADDR_LSB_BIT + CFG_SENSOR_MUX_WIDTH) -\
    _BV(CFG_SENSOR_MUX_ADDR_LSB_BIT))

static uint8_t last_sensor;

static inline void _select_plant(uint8_t plant_idx)
{
    CFG_SENSOR_MUX_PORT &= ~MUX_MASK;
    CFG_SENSOR_MUX_PORT |= (plant_idx << CFG_SENSOR_MUX_ADDR_LSB_BIT) |
        _BV(CFG_SENSOR_MUX_ENABLE_BIT); 
}

/**
 * sensors_start_measurement - Selects sensor and starts measuring
 * @plant_idx: The plant index
 * @what: What to measure (a SENSOR_* constant)
 */
void sensors_start_measurement(uint8_t plant_idx, uint8_t what)
{   
    last_sensor = what;
    
    _select_plant(plant_idx);
    
    adc_set_prescaler(CFG_SENSOR_ADC_PRESCALER);
    adc_set_samples(CFG_SENSOR_ADC_SAMPLES);
    adc_set_reference(ADC_REF_VCC);
    adc_set_channel(what);
    adc_start();
}

/**
 * sensors_ready - Checks that measurement is complete
 */
bool sensors_ready(void)
{
    return adc_is_ready();
}

/**
 * sensors_get - Get measured value
 */
uint16_t sensors_get(void)
{
    uint16_t raw_val, calib_min=0, calib_max=1023, temp;
    bool reverse;
    
    CFG_SENSOR_MUX_PORT &= ~_BV(CFG_SENSOR_MUX_ENABLE_BIT);
    
    raw_val = adc_get();
    
    switch (last_sensor) {
        case SENSOR_HUMID:
            calib_min = CFG_SENSOR_CALIB_HUMID_MIN;
            calib_max = CFG_SENSOR_CALIB_HUMID_MAX;
            break;
        case SENSOR_ILUM:
            calib_min = CFG_SENSOR_CALIB_ILUM_MIN;
            calib_max = CFG_SENSOR_CALIB_ILUM_MAX;
            break;
    }
    
    reverse = FALSE;
    if (calib_min > calib_max) {
        reverse = TRUE;
        temp = calib_min;
        calib_min = calib_max;
        calib_max = temp;
    }
    
    if (raw_val < calib_min) {
        raw_val = calib_min;
    }
    if (raw_val > calib_max) {
        raw_val = calib_max;
    }
    
    raw_val = (uint32_t)(raw_val - calib_min) * 100 / (calib_max - calib_min);
    
    if (reverse) {
        raw_val = 100 - raw_val;
    }
    
    return raw_val;
}

/**
 * sensors_init - Initializes sensors
 */
void sensors_init(void)
{
    CFG_SENSOR_MUX_DDR |= _BV(CFG_SENSOR_MUX_ENABLE_BIT) | MUX_MASK;
    CFG_SENSOR_MUX_PORT &= ~MUX_MASK;
    CFG_SENSOR_MUX_PORT &= ~_BV(CFG_SENSOR_MUX_ENABLE_BIT);

    DDRA &= ~(_BV(CFG_SENSOR_HUMID_CHAN) + _BV(CFG_SENSOR_ILUM_CHAN));
}
