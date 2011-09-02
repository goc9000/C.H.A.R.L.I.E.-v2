/*
 * tcp.h - TCP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PROTO_TCP_H
#define _PROTO_TCP_H

#include <inttypes.h>

#include "net/proto/ip.h"
#include "net/packet_buf.h"
#include "net/endian.h"
#include "util/bool.h"

#define TCP_FLAG_FIN  1
#define TCP_FLAG_SYN  2
#define TCP_FLAG_RST  4
#define TCP_FLAG_PSH  8
#define TCP_FLAG_ACK  16
#define TCP_FLAG_URG  32
#define TCP_FLAG_ECE  64
#define TCP_FLAG_CWR  128

typedef struct {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t seq_no;
    uint32_t ack_no;
    uint8_t  reserved:4;
    uint8_t  ihl:4;
    uint8_t  flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_ptr;
} tcp_Header;

#define EC_SIG_TCP \
    DEF_EC_FIELD(tcp_Header, source_port, 2) + \
    DEF_EC_FIELD(tcp_Header, dest_port, 2) + \
    DEF_EC_FIELD(tcp_Header, seq_no, 4) + \
    DEF_EC_FIELD(tcp_Header, ack_no, 4) + \
    DEF_EC_FIELD(tcp_Header, window_size, 2) + \
    DEF_EC_FIELD(tcp_Header, checksum, 2) + \
    DEF_EC_FIELD(tcp_Header, urgent_ptr, 2)

bool tcp_is_connection_begin_seg(const tcp_Header *header);

void tcp_get_header(PacketBuf *packet, tcp_Header *tcp_hdr);
void tcp_update_hdr_checksum(tcp_Header *header, PacketBuf *payload,
    const ip_addr_t *source_ip, const ip_addr_t *dest_ip);
void tcp_make_reply_header(tcp_Header *header, tcp_Header *request,
    uint32_t seq_no, uint32_t ack_no, uint8_t flags);

#endif
