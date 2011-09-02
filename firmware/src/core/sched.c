/*
 * sched.c - Task scheduling functionality
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <string.h>

#include "time/time.h"
#include "sched.h"

struct {
    Task queue[CFG_SCHED_SLOTS];
    int n_tasks;
} sched;

static void _sched_unschedule(callback_t callback, uint32_t param,
    bool use_param);
static bool _sched_is_scheduled(callback_t callback, uint32_t param,
    bool use_param);

void _sched_delete(uint8_t index)
{
    memcpy(sched.queue + index, sched.queue + index + 1,
                sizeof(Task)*(sched.n_tasks - index - 1));
    sched.n_tasks--;
}

static bool _sched_schedule(uint32_t delta, callback_t callback,
    uint32_t param, bool use_param)
{
    Task task;
    uint8_t pos;
    
    if (_sched_is_scheduled(callback, param, use_param)) {
        _sched_unschedule(callback, param, use_param);
    }
    
    if (sched.n_tasks == CFG_SCHED_SLOTS) {
        return FALSE;
    }
    
    task.time = time_get_uptime() + delta;
    task.callback = callback;
    task.param = param;
    pos = sched.n_tasks++;
    
    while (pos > 0) {
        if (time_compare_monotonic(task.time, sched.queue[pos-1].time) >= 0) {
            break;
        }
        
        sched.queue[pos] = sched.queue[pos-1];
        pos--;
    }
    
    sched.queue[pos] = task;
    
    return TRUE;
}

static void _sched_unschedule(callback_t callback, uint32_t param,
    bool use_param)
{
    uint8_t i;
    for (i = 0; i < sched.n_tasks; i++) {
        if ((sched.queue[i].callback == callback) &&
            (!use_param || (sched.queue[i].param == param))) {
            _sched_delete(i);
            return;
        }
    }
}

static bool _sched_is_scheduled(callback_t callback, uint32_t param,
    bool use_param)
{
    uint8_t i;
    for (i = 0; i < sched.n_tasks; i++) {
        if ((sched.queue[i].callback == callback) &&
            (!use_param || (sched.queue[i].param == param))) {
            return TRUE;
        }
    }
    
    return FALSE;
}

/**
 * sched_schedule - Schedules a task to be run in the future
 * @delta: The time after which the task will be executed, in centis
 * @callback: The callback to be run
 * 
 * Note: if the task is already present in the queue, it will be
 * rescheduled. Returns FALSE if there was no more room in the queue.
 */
bool sched_schedule(uint32_t delta, callback_t callback)
{
    return _sched_schedule(delta, callback, 0, FALSE);
}

/**
 * sched_schedule_param - Schedules a task to be run in the future
 * @delta: The time after which the task will be executed, in centis
 * @callback: The callback to be run
 * @param: A parameter passed to the callback
 * 
 * Note: if the task is already present in the queue, it will be
 * rescheduled. Returns FALSE if there was no more room in the queue.
 */
bool sched_schedule_param(uint32_t delta, callback_t callback, uint32_t param)
{
    return _sched_schedule(delta, callback, param, TRUE);
}

/**
 * sched_unschedule - Unschedules a task
 * @callback: The callback that identifies the task
 */
void sched_unschedule(callback_t callback)
{
    _sched_unschedule(callback, 0, FALSE);
}

/**
 * sched_unschedule_param - Unschedules a task
 * @callback: The callback that identifies the task
 * @param: The parameter that identifies the task
 */
void sched_unschedule_param(callback_t callback, uint32_t param)
{
    _sched_unschedule(callback, param, TRUE);
}

/**
 * sched_is_scheduled - Checks whether a task is currently scheduled
 * @callback: The callback that identifies the task
 */
bool sched_is_scheduled(callback_t callback)
{
    return _sched_is_scheduled(callback, 0, FALSE);
}

/**
 * sched_is_scheduled - Checks whether a task is currently scheduled
 * @callback: The callback that identifies the task
 * @param: The parameter that identifies the task
 */
bool sched_is_scheduled_param(callback_t callback, uint32_t param)
{
    return _sched_is_scheduled(callback, param, TRUE);
}

/**
 * sched_run - Run any tasks scheduled at or before the current time
 */
void sched_run(void)
{
    uint32_t now;
    Task task;

    while (sched.n_tasks) {
        now = time_get_uptime();
        if (time_compare_monotonic(now, sched.queue[0].time) < 0) {
            break;
        }
        
        task = sched.queue[0];
        _sched_delete(0);
        
        task.callback(task.param);
    }
}

/**
 * sched_time_to_next - Gets the time to the next scheduled event
 */
uint32_t sched_time_to_next(void)
{
    uint32_t now = time_get_uptime();

    if (!sched.n_tasks) {
        return SCHED_TIME_INF;
    }
    
    if (time_compare_monotonic(now, sched.queue[0].time) > 0) {
        return 0;
    }
    
    return sched.queue[0].time - now;
}

/**
 * sched_init - Initializes the scheduler
 */
void sched_init(void)
{
    sched.n_tasks = 0;
}
