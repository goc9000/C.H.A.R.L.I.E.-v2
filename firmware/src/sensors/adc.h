/*
 * adc.h - Analog-to-digital converter functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _ADC_H
#define _ADC_H

#include <inttypes.h>
#include <avr/io.h>

#include "util/bool.h"

#define ADC_REF_VCC (_BV(REFS0))
#define ADC_REF_256 (_BV(REFS0) + _BV(REFS1))
#define ADC_REF_EXT (0)

void adc_set_prescaler(uint8_t pow2);
void adc_set_channel(uint8_t index);
void adc_set_reference(uint8_t ref);
void adc_set_samples(uint16_t samples);
void adc_start(void);
bool adc_is_ready(void);
uint16_t adc_get(void);
void adc_init(void);

#endif
