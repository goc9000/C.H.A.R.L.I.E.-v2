/*
 * eth_impl.c - Ethernet layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "debug/debug.h"
#include "net/dev/enc28j60.h"
#include "ip_impl.h"
#include "arp_impl.h"
#include "eth_impl.h"

/**
 * eth_receive - Handles the receipt of an Ethernet frame
 * @packet: A packet buffer containing the frame
 */
void eth_receive(PacketBuf *packet)
{
    eth_Header header;
    
    pktbuf_get_header(packet, &header, sizeof(eth_Header), EC_SIG_ETH);
    
    switch (header.ether_type)
    {
        case ETHERTYPE_IPV4:
            ip_receive(packet, &header);
            break;
        case ETHERTYPE_ARP:
            arp_receive(packet);
            break;
    }
}

/**
 * eth_send - Sends an Ethernet frame
 * @packet: A packet buffer with the frame contents
 * @dest_mac: The destination MAC address
 * @ether_type: The EtherType value
 */
void eth_send(PacketBuf* packet, const mac_addr_t *dest_mac,
    uint16_t ether_type)
{
    eth_Header header;
    
    header.dest_mac = *dest_mac;
    enc28j60_get_mac_addr(&header.source_mac);
    header.ether_type = ether_type;
    
    pktbuf_add_header(packet, &header, sizeof(eth_Header), EC_SIG_ETH);
    enc28j60_transmit(packet);
    pktbuf_pull(packet, sizeof(eth_Header));
}
