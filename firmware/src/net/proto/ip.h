/*
 * ip.h - IP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PROTO_IP_H
#define _PROTO_IP_H

#include <inttypes.h>

#include "net/packet_buf.h"
#include "net/endian.h"
#include "util/bool.h"

#define IP_PROTO_ICMP          1
#define IP_PROTO_TCP           6
#define IP_PROTO_UDP           17

#define IP_FRAG_OFFS_MASK      0x1FFF
#define IP_FRAG_DONT_FRAGMENT  0x4000
#define IP_FRAG_MORE_FRAGMENTS 0x2000

typedef struct {
    uint8_t o0,o1,o2,o3;
} ip_addr_t;

typedef struct {
    uint8_t   ihl:4;
    uint8_t   version:4;
    uint8_t   tos;
    uint16_t  length;
    uint16_t  ident;
    uint16_t  fragments;
    uint8_t   ttl;
    uint8_t   protocol;
    uint16_t  checksum;
    ip_addr_t source_ip;
    ip_addr_t dest_ip;
} ip_Header;

#define EC_SIG_IP \
    DEF_EC_FIELD(ip_Header,length,2) + \
    DEF_EC_FIELD(ip_Header,ident,2) + \
    DEF_EC_FIELD(ip_Header,fragments,2) + \
    DEF_EC_FIELD(ip_Header,checksum,2)

extern ip_addr_t IP_ADDR_NULL;
extern ip_addr_t IP_ADDR_BCAST;

bool ip_is_broadcast_ip(const ip_addr_t *addr, uint8_t net_mask);
bool ip_addr_equal(const ip_addr_t *addr1, const ip_addr_t *addr2);
bool ip_addr_match(const ip_addr_t *addr1, const ip_addr_t *addr2,
    uint8_t net_mask);
bool ip_same_network(const ip_addr_t *addr1, const ip_addr_t *addr2,
    uint8_t net_mask);

void ip_make_bcast_addr(ip_addr_t *bcast_addr,
    const ip_addr_t *base_addr, uint8_t net_mask);

uint8_t ip_get_mask_byte(uint8_t mask, uint8_t index);
uint8_t ip_decode_net_mask(const ip_addr_t *net_mask);

void ip_format(char *buf, const ip_addr_t *addr);
bool ip_parse(ip_addr_t *addr, const char *text);

uint16_t sum1c(uint16_t a, uint16_t b);
uint16_t ip_checksum(const void *data, uint16_t data_size);
uint16_t ip_join_checksums(uint16_t sum_a, uint16_t sum_b);
uint16_t ip_update_checksum(uint16_t checksum, uint16_t old_field_val,
    uint16_t new_field_val);

void ip_get_header(PacketBuf *packet, ip_Header *ip_hdr);
void ip_update_hdr_checksum(ip_Header *header);
bool ip_is_fragment(const ip_Header *header);

#endif
