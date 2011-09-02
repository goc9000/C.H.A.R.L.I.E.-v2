/*
 * arp_impl.h - ARP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _INCLUDED_NET_ARP_H
#define _INCLUDED_NET_ARP_H

#include <inttypes.h>

#include "net/proto/eth.h"
#include "net/proto/ip.h"
#include "net/proto/arp.h"
#include "util/bool.h"

typedef struct {
    ip_addr_t ip_addr;
    mac_addr_t mac_addr;
} arp_TableEntry;

void arp_receive(PacketBuf *packet);
void arp_transmit(arp_PacketV4 *arp_pkt);

void arp_learn(const ip_addr_t *ip_addr, const mac_addr_t *mac_addr);
bool arp_lookup(const ip_addr_t *ip_addr, mac_addr_t *mac_addr);
void arp_init(void);

#endif
