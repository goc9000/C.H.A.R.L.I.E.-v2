/*
 * dhcp.h - DHCP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PROTO_DHCP_H
#define _PROTO_DHCP_H

#include <inttypes.h>

#include "net/packet_buf.h"
#include "net/endian.h"
#include "net/proto/eth.h"
#include "net/proto/ip.h"

#define DHCP_OP_REQUEST        1
#define DHCP_OP_REPLY          2

#define DHCP_HW_TYPE_ETHERNET  0x01

#define DHCP_PKT_DISCOVER       1
#define DHCP_PKT_OFFER          2
#define DHCP_PKT_REQUEST        3
#define DHCP_PKT_DECLINE        4
#define DHCP_PKT_ACK            5
#define DHCP_PKT_NAK            6
#define DHCP_PKT_RELEASE        7

#define DHCP_OPTION_PAD         0
#define DHCP_OPTION_NETMASK     1
#define DHCP_OPTION_GATEWAY     3
#define DHCP_OPTION_PKT_TYPE    53
#define DHCP_OPTION_SERVER_ID   54
#define DHCP_OPTION_RENEW_TIME  58
#define DHCP_OPTION_END         255

#define DHCP_SERVER_PORT        67
#define DHCP_CLIENT_PORT        68

typedef struct {
    uint8_t    opcode;
    uint8_t    hardw_type;
    uint8_t    hardw_addr_len;
    uint8_t    hops;
    uint32_t   transaction_id;
    uint16_t   seconds;
    uint16_t   flags;
    ip_addr_t  client_ip;
    ip_addr_t  your_ip;
    ip_addr_t  next_server_ip;
    ip_addr_t  relay_ip;
    mac_addr_t client_hw_addr;
} dhcp_Header;

#define EC_SIG_DHCP \
    DEF_EC_FIELD(dhcp_Header, transaction_id, 4) + \
    DEF_EC_FIELD(dhcp_Header, seconds, 2) + \
    DEF_EC_FIELD(dhcp_Header, flags, 2)

void dhcp_make_packet(PacketBuf *packet, dhcp_Header *header,
    uint8_t dhcp_type);
uint32_t dhcp_make_discover_packet(PacketBuf *packet,
    const mac_addr_t *my_mac);
void dhcp_make_accept_packet(PacketBuf *packet,
    const dhcp_Header *offer_header, const ip_addr_t *server_id);
void dhcp_make_renew_packet(PacketBuf *packet, uint32_t tran_id,
    const mac_addr_t *my_mac, const ip_addr_t *my_ip);
void dhcp_add_option(PacketBuf *packet, uint8_t option, uint8_t length,
    const void *data);
void dhcp_parse_options(PacketBuf *packet, uint8_t *pkt_type,
    ip_addr_t *server_id, ip_addr_t *gateway, ip_addr_t *net_mask,
    uint32_t *renew_time);

#endif
