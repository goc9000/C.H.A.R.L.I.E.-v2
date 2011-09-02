/*
 * ip_impl.h - IP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _IP_IMPL_H
#define _IP_IMPL_H

#include <inttypes.h>
#include "proto/eth.h"
#include "proto/ip.h"
#include "packet_buf.h"

void ip_receive(PacketBuf *packet, eth_Header *eth_hdr);
void ip_send(PacketBuf *packet, const ip_addr_t *dest_ip,
    uint16_t ip_proto);

#endif
