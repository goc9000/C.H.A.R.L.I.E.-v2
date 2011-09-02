/*
 * sensors.h - Sensors subsystem functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _SENSORS_H
#define _SENSORS_H

#include <inttypes.h>

#include "util/bool.h"
#include "dot_config.h"

#define SENSOR_HUMID  CFG_SENSOR_HUMID_CHAN
#define SENSOR_ILUM   CFG_SENSOR_ILUM_CHAN

void sensors_init(void);
void sensors_start_measurement(uint8_t plant_idx, uint8_t what);
bool sensors_ready(void);
uint16_t sensors_get(void);

#endif
