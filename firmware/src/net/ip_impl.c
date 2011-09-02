/*
 * ip_impl.c - IP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <string.h>

#include "debug/debug.h"
#include "net_core.h"
#include "eth_impl.h"
#include "ip_impl.h"
#include "arp_impl.h"
#include "udp_impl.h"
#include "tcp_impl.h"

/**
 * ip_receive - Handles the receipt of an IP packet
 * @packet: A packet buffer containing the packet
 * @eth_hdr: The extracted Ethernet header
 */
void ip_receive(PacketBuf *packet, eth_Header *eth_hdr)
{
    ip_Header ip_hdr;
    
    ip_get_header(packet, &ip_hdr);
    
    if (!net_is_for_my_ip(&ip_hdr.dest_ip)) {
        return;
    }

    // only receive first fragment
    if (ip_hdr.fragments & IP_FRAG_OFFS_MASK) {
        return;
    }
    
    arp_learn(&(ip_hdr.source_ip), &eth_hdr->source_mac);

    switch (ip_hdr.protocol) {
        case IP_PROTO_UDP:
            udp_receive(packet, &ip_hdr);
            break;
        case IP_PROTO_TCP:
            tcp_receive(packet, &ip_hdr);
            break;
    }
}

/**
 * ip_send - Sends an IP packet
 * @packet: A packet buffer with the packet contents
 * @dest_ip: The destination IP address
 * @ip_proto: The encapsulated protocol value
 */
void ip_send(PacketBuf *packet, const ip_addr_t *dest_ip,
    uint16_t ip_proto)
{
    ip_Header ip_hdr;
    mac_addr_t dest_mac;
    
    arp_lookup(dest_ip, &dest_mac);

    memset(&ip_hdr, 0, sizeof(ip_Header));
    ip_hdr.ihl = 5;
    ip_hdr.length = sizeof(ip_Header) + packet->length;
    ip_hdr.version = 4;
    ip_hdr.ttl = 255;
    ip_hdr.protocol = ip_proto;
    net_get_config(NULL, &ip_hdr.source_ip, NULL, NULL);
    ip_hdr.dest_ip = *dest_ip;
    ip_update_hdr_checksum(&ip_hdr);
    
    pktbuf_add_header(packet, &ip_hdr, sizeof(ip_Header), EC_SIG_IP);
    eth_send(packet, &dest_mac, ETHERTYPE_IPV4);
    pktbuf_pull(packet, sizeof(ip_Header));
}
