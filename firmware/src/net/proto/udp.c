/*
 * udp.c - UDP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "udp.h"

/**
 * udp_get_header - Extract UDP header from packet
 * @packet: A packet buffer containing an UDP packet
 * @ip_hdr: A structure to store the extracted header
 */
void udp_get_header(PacketBuf *packet, udp_Header *udp_hdr)
{
    pktbuf_get_header(packet, udp_hdr, sizeof(udp_Header), EC_SIG_UDP);
    pktbuf_trim(packet, udp_hdr->length - sizeof(udp_Header));
}
