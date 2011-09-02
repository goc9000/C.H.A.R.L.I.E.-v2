/*
 * net_core.h - Core network functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _NET_CORE_H
#define _NET_CORE_H

#include <inttypes.h>

#include "proto/eth.h"
#include "proto/ip.h"
#include "util/bool.h"

void net_set_config(const ip_addr_t *ip, uint8_t net_mask,
    const ip_addr_t *gateway_ip);
void net_get_config(mac_addr_t *mac_addr, ip_addr_t *ip_addr,
    uint8_t *net_mask, ip_addr_t *gw_addr);
bool net_is_for_my_ip(const ip_addr_t *ip_addr);
bool net_is_broadcast_ip(const ip_addr_t *ip_addr);
bool net_is_in_local_net(const ip_addr_t *ip_addr);
void net_receive(uint32_t dummy __attribute__((unused)));
bool net_busy(void);

void net_init(void);

#endif
