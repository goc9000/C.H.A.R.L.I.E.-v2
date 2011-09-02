/*
 * dhcp_impl.h - DHCP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _DHCP_IMPL_H
#define _DHCP_IMPL_H

#include <inttypes.h>

#include "proto/ip.h"
#include "proto/dhcp.h"
#include "packet_buf.h"

void dhcp_receive(PacketBuf *packet, ip_Header *ip_hdr);
void dhcp_send(PacketBuf *packet, const ip_addr_t *dest_ip);

void dhcp_start_client(uint32_t dummy __attribute__((unused)));

void dhcp_init(void);

#endif
