/*
 * udp_impl.c - UDP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _UDP_IMPL_H
#define _UDP_IMPL_H

#include <inttypes.h>

#include "proto/eth.h"
#include "proto/ip.h"

void udp_receive(PacketBuf *packet, ip_Header *ip_hdr);
void udp_send(PacketBuf *packet, const ip_addr_t *dest_ip,
    uint16_t src_port, uint16_t dest_port);

#endif
