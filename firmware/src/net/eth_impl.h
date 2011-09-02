/*
 * eth_impl.h - Ethernet layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _ETH_IMPL_H
#define _ETH_IMPL_H

#include <inttypes.h>
#include "proto/eth.h"
#include "packet_buf.h"

void eth_receive(PacketBuf *packet);
void eth_send(PacketBuf* packet, const mac_addr_t *dest_mac,
    uint16_t ether_type);

#endif
