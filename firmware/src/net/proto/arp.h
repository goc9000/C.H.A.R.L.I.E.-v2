/*
 * arp.h - ARP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PROTO_ARP_H
#define _PROTO_ARP_H

#include <inttypes.h>

#include "net/proto/eth.h"
#include "net/proto/ip.h"
#include "net/endian.h"

#define ARP_OP_REQUEST        1
#define ARP_OP_REPLY          2

#define ARP_HW_TYPE_ETHERNET  0x0001

#define ARP_V4_SIZE           28

typedef struct {
    uint16_t   hardw_type;
    uint16_t   proto_type;
    uint8_t    hardw_addr_len;
    uint8_t    proto_addr_len;
    uint16_t   operation;
    mac_addr_t sender_mac;
    ip_addr_t  sender_ip;
    mac_addr_t target_mac;
    ip_addr_t  target_ip;
} arp_PacketV4;

#define EC_SIG_ARPV4 \
    DEF_EC_FIELD(arp_PacketV4,hardw_type,2) + \
    DEF_EC_FIELD(arp_PacketV4,proto_type,2) + \
    DEF_EC_FIELD(arp_PacketV4,operation,2)

void arp_make_query_packet(arp_PacketV4* arp_pkt, const ip_addr_t *for_ip,
    const ip_addr_t *my_ip, const mac_addr_t *my_mac);
void arp_make_reply_packet(arp_PacketV4 *arp_pkt, arp_PacketV4 *for_pkt,
    const ip_addr_t *my_ip, const mac_addr_t *my_mac);
    
#endif
