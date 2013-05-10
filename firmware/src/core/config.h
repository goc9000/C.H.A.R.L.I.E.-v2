/*
 * config.h - Config file functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include <inttypes.h>
#include "model/records.h"
#include "net/proto/eth.h"
#include "net/proto/ip.h"
#include "dot_config.h"

#define PLANT_CFG_FLAG_NOT_INSTALLED 1

typedef struct {
    uint8_t watering_start_threshold; // 0-100 for 0-100%
    uint8_t watering_stop_threshold; // 0-100 for 0-100%
    uint8_t flags;
} PlantConfig;

typedef struct {
    PlantConfig plants[CFG_MAX_PLANTS];
    mac_addr_t mac_addr;
    ip_addr_t alerts_server_ip;
    uint16_t alerts_port;
    ip_addr_t time_server_ip;
    int16_t timezone_delta;
    uint16_t recording_interval;
} Config;

extern Config cfg;

void cfg_load(void);
void cfg_save(void);

#endif
