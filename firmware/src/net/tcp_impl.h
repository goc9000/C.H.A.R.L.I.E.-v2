/*
 * tcp_impl.h - TCP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _TCP_IMPL_H
#define _TCP_IMPL_H

#include <inttypes.h>

#include "net/proto/ip.h"
#include "net/proto/tcp.h"
#include "packet_buf.h"
#include "util/bool.h"

void tcp_receive(PacketBuf *packet, ip_Header *ip_hdr);
void tcp_send(PacketBuf *packet, tcp_Header *tcp_hdr, const ip_addr_t *dest_ip);

bool tcp_port_is_open(uint16_t port);
void tcp_start_request(uint16_t port);
bool tcp_receive_request(PacketBuf* data, uint16_t port);
bool tcp_get_answer(PacketBuf* data, uint16_t port);
bool tcp_busy(void);

void tcp_init(void);

#endif
