/*
 * eth.h - Ethernet protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */
 
#ifndef _PROTO_ETH_H
#define _PROTO_ETH_H

#include <inttypes.h>

#include "net/endian.h"
#include "util/bool.h"

// EtherType constants
#define ETHERTYPE_RAW    0x0000
#define ETHERTYPE_IPV4   0x0800
#define ETHERTYPE_ARP    0x0806
#define ETHERTYPE_MAGIC  0x0842
#define ETHERTYPE_RARP   0x8035
#define ETHERTYPE_VLAN   0x8100
#define ETHERTYPE_8021Q  0x8100

typedef struct {
    uint8_t o0,o1,o2,o3,o4,o5;
} mac_addr_t;

typedef struct {
    mac_addr_t dest_mac;
    mac_addr_t source_mac;
    uint16_t   ether_type;
} eth_Header;

#define EC_SIG_ETH \
    DEF_EC_FIELD(eth_Header,ether_type,2)

extern mac_addr_t MAC_ADDR_NULL;
extern mac_addr_t MAC_ADDR_BCAST;

bool eth_addr_equal(const mac_addr_t *addr1, const mac_addr_t *addr2);
bool eth_addr_match(const mac_addr_t *addr1, const mac_addr_t *addr2);
void eth_format_mac(char *buf, const mac_addr_t *addr);
bool eth_parse_mac(mac_addr_t *addr, const char *text);

#endif
