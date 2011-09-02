/*
 * sched.h - Task scheduling functionality
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _SCHED_H
#define _SCHED_H

#include <inttypes.h>

#include "util/bool.h"
#include "dot_config.h"

typedef void (*callback_t)(uint32_t);

#define SCHED_TIME_INF  0xFFFFFFFF
#define SCHED_SEC       100

typedef struct {
    uint32_t time;
    callback_t callback;
    uint32_t param;
} Task;

void sched_init(void);
bool sched_schedule(uint32_t delta, callback_t callback);
bool sched_schedule_param(uint32_t delta, callback_t callback, uint32_t param);
void sched_unschedule(callback_t callback);
void sched_unschedule_param(callback_t callback, uint32_t param);
bool sched_is_scheduled(callback_t callback);
bool sched_is_scheduled_param(callback_t callback, uint32_t param);
void sched_run(void);
uint32_t sched_time_to_next(void);

#endif
