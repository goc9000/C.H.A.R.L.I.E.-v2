/*
 * udp_impl.c - UDP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "proto/udp.h"
#include "ip_impl.h"
#include "udp_impl.h"
#include "dhcp_impl.h"

/**
 * udp_receive - Handles the receipt of an UDP packet
 * @packet: A packet buffer containing the packet
 * @ip_hdr: The extracted IP header
 */
void udp_receive(PacketBuf *packet, ip_Header *ip_hdr)
{
    udp_Header udp_hdr;
    
    pktbuf_get_header(packet, &udp_hdr, sizeof(udp_Header), EC_SIG_UDP);
    
    switch (udp_hdr.dest_port) {
        case DHCP_CLIENT_PORT:
            dhcp_receive(packet, ip_hdr);
            break;
    }
}

/**
 * udp_send - Sends an UDP packet
 * @packet: A packet buffer with the packet contents
 * @dest_ip: The destination IP address
 * @src_port: The source port
 * @dest_port: The destination port
 */
void udp_send(PacketBuf *packet, const ip_addr_t *dest_ip,
    uint16_t src_port, uint16_t dest_port)
{
    udp_Header udp_hdr;
    
    udp_hdr.source_port = src_port;
    udp_hdr.dest_port = dest_port;
    udp_hdr.checksum = 0;
    udp_hdr.length = sizeof(udp_Header) + packet->length;
    
    pktbuf_add_header(packet, &udp_hdr, sizeof(udp_Header), EC_SIG_UDP);
    ip_send(packet, dest_ip, IP_PROTO_UDP);
    pktbuf_pull(packet, sizeof(udp_Header));
}
