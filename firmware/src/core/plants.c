/*
 * plants.c - Plant watering & monitoring
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "model/records.h"
#include "model/log.h"
#include "sensors/sensors.h"
#include "effectors/pump.h"
#include "core/config.h"
#include "core/sched.h"
#include "plants.h"

#define ILUM_SENSOR_CHECK_PERIOD 5
#define HUMID_SENSOR_CHECK_PERIOD 5
#define WATERED_CHECK_PERIOD 20

struct {
    uint8_t plant_idx;
    Record current; 
    Record last;
} plants;

static void _plants_handle_plant(uint32_t dummy __attribute__((unused)));
static void _plants_start_record(uint32_t dummy __attribute__((unused)));

static void _plants_end_record(uint32_t dummy __attribute__((unused)))
{
    plants.last = plants.current;
    rec_save_record(&plants.current);
    
    sched_schedule(SCHED_SEC * cfg.recording_interval, _plants_start_record);
}

static void _plants_next_plant(uint32_t dummy __attribute__((unused)))
{
    plants.plant_idx++;
    
    if (plants.plant_idx >= CFG_MAX_PLANTS) {
        sched_schedule(0, _plants_end_record);
        return;
    }
    
    sched_schedule(0, _plants_handle_plant);
}

static void _plants_check_watered(uint32_t dummy __attribute__((unused)))
{
    uint8_t humidity;
    
    if (!sensors_ready()) {
        sched_schedule(WATERED_CHECK_PERIOD, _plants_check_watered);
        return;
    }
    
    humidity = sensors_get();
    
    if (humidity < cfg.plants[plants.plant_idx].watering_stop_threshold) {
        sched_schedule(WATERED_CHECK_PERIOD, _plants_check_watered);
        return;
    }
    
    pump_stop();
    plants.current.plants[plants.plant_idx].humidity = humidity;
    log_make_entry(LOG_EVENT_DONE_WATERING, plants.plant_idx);
    
    sched_schedule(0, _plants_next_plant);
}

static void _plants_start_watering(uint32_t dummy __attribute__((unused)))
{
    pump_start(plants.plant_idx);
    sensors_start_measurement(plants.plant_idx, SENSOR_HUMID);
    log_make_entry(LOG_EVENT_WATERING_PLANT, plants.plant_idx);
    
    sched_schedule(WATERED_CHECK_PERIOD, _plants_check_watered);
}

static void _plants_check_humid_ready(uint32_t dummy __attribute__((unused)))
{
    uint8_t humidity;
    
    if (!sensors_ready()) {
        sched_schedule(HUMID_SENSOR_CHECK_PERIOD, _plants_check_humid_ready);
        return;
    }
    
    humidity = sensors_get();
    
    if (humidity < cfg.plants[plants.plant_idx].watering_start_threshold) {
        sched_schedule(0, _plants_start_watering);
        return;
    }
    
    plants.current.plants[plants.plant_idx].humidity = humidity;
    
    sched_schedule(0, _plants_next_plant);
}

static void _plants_meas_humid(uint32_t dummy __attribute__((unused)))
{
    sensors_start_measurement(plants.plant_idx, SENSOR_HUMID);
    
    sched_schedule(HUMID_SENSOR_CHECK_PERIOD, _plants_check_humid_ready);
}

static void _plants_check_ilum_ready(uint32_t dummy __attribute__((unused)))
{
    if (!sensors_ready()) {
        sched_schedule(ILUM_SENSOR_CHECK_PERIOD, _plants_check_ilum_ready);
        return;
    }
    
    plants.current.plants[plants.plant_idx].ilumination = sensors_get();
    
    sched_schedule(0, _plants_meas_humid);
}

static void _plants_meas_ilum(uint32_t dummy __attribute__((unused)))
{
    sensors_start_measurement(plants.plant_idx, SENSOR_ILUM);
    
    sched_schedule(ILUM_SENSOR_CHECK_PERIOD, _plants_check_ilum_ready);
}

static void _plants_handle_plant(uint32_t dummy __attribute__((unused)))
{
    plants.current.plants[plants.plant_idx].flags = 0;
    
    if (cfg.plants[plants.plant_idx].flags & PLANT_CFG_FLAG_NOT_INSTALLED) {
        plants.current.plants[plants.plant_idx].flags |=
            PLANT_FLAGS_NOT_INSTALLED;
        sched_schedule(0, _plants_next_plant);
        return;
    }
    
    sched_schedule(0, _plants_meas_ilum);
}

static void _plants_start_record(uint32_t dummy __attribute__((unused)))
{
    plants.current.time = time_get_raw();
    plants.plant_idx = 0;
    
    sched_schedule(0, _plants_handle_plant);
}

/**
 * plants_get_latest_record - Gets the latest complete record produced
 *                            by the plants module
 * @record: A variable to receive the record
 */
void plants_get_latest_record(Record *record)
{
    *record = plants.last;
}

/**
 * plants_busy - Checks whether we are busy with some important action
 *               in the plants module
 */
bool plants_busy(void)
{
    return !sched_is_scheduled(_plants_start_record);
}

/**
 * plants_init - Initializes the plant caretaking subsystem
 */
void plants_init(void)
{
    sched_schedule(0, _plants_start_record);
}
