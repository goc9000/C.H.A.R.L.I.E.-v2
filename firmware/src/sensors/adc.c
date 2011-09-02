/*
 * adc.c - Analog-to-digital converter functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc.h"

static volatile struct {
    uint32_t sample_sum;
    uint16_t sample_count;
    uint16_t sample_target;
    bool ready;
} adc;

/**
 * ADC conversion complete interrupt handler
 */
ISR(ADC_vect)
{
    uint16_t sample = ADC;
    
    adc.sample_sum += sample;
    adc.sample_count++;
    
    if (adc.sample_count >= adc.sample_target) {
        adc.ready = TRUE;
        ADCSRA &= ~_BV(ADEN);
        return;
    }
    
    ADCSRA |= _BV(ADSC);
}

/**
 * adc_set_prescaler - Sets the prescaler on the ADC
 * @pow2: The exponent in speed = max/2^x
 */
void adc_set_prescaler(uint8_t pow2)
{
    ADCSRA = (ADCSRA & 0xF8) | pow2;
}

/**
 * adc_set_channel - Sets the channel source for measurement
 * @index: The channel index (0-7)
 */
void adc_set_channel(uint8_t index)
{
    ADMUX = ((ADMUX & 0xE0) | index);
}

/**
 * adc_set_reference - Sets the conversion reference
 * @ref: An ADC_REF_* constant
 */
void adc_set_reference(uint8_t ref)
{
    ADMUX = ((ADMUX & 0x3F) | ref);
}

/**
 * adc_set_samples - Sets the no. of samples averaged for a conversion
 * @samples: The number of samples
 */
void adc_set_samples(uint16_t samples)
{
    adc.sample_target = samples;
}

/**
 * adc_start - Starts the measurement
 */
void adc_start(void)
{
    adc.sample_sum = 0;
    adc.sample_count = 0;
    adc.ready = FALSE;
    
    ADCSRA |= _BV(ADEN) + _BV(ADSC);
}

/**
 * adc_is_ready - Checks whether conversion is complete
 */
bool adc_is_ready(void)
{
    return adc.ready;
}

/**
 * adc_get - Gets the measured and averaged value
 */
uint16_t adc_get(void)
{
    return (adc.sample_count) ? (adc.sample_sum / adc.sample_count) : 0;
}

/**
 * adc_init - Initializes the ADC
 */
void adc_init(void)
{
    ADMUX = 0;
    ADCSRA = _BV(ADIE);

    adc.ready = TRUE;
}
