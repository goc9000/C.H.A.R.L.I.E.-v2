/*
 * udp.h - UDP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PROTO_UDP_H
#define _PROTO_UDP_H

#include <inttypes.h>

#include "net/packet_buf.h"
#include "net/endian.h"

typedef struct {
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t checksum;
} udp_Header;

#define EC_SIG_UDP \
    DEF_EC_FIELD(udp_Header,source_port,2) + \
    DEF_EC_FIELD(udp_Header,dest_port,2) + \
    DEF_EC_FIELD(udp_Header,length,2) + \
    DEF_EC_FIELD(udp_Header,checksum,2)

void udp_get_header(PacketBuf *packet, udp_Header *udp_hdr);

#endif
