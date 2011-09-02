/*
 * arp.c - ARP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <string.h>

#include "arp.h"

/**
 * arp_make_query_packet - Creates an ARP query packet
 * @arp_pkt: A structure to receive the packet data
 * @for_ip: The IP for which we wish to find the MAC address
 * @my_ip: Our own IP
 * @my_mac: Our own MAC address
 */
void arp_make_query_packet(arp_PacketV4* arp_pkt, const ip_addr_t *for_ip,
    const ip_addr_t *my_ip, const mac_addr_t *my_mac)
{
    arp_pkt->hardw_type     = ARP_HW_TYPE_ETHERNET;
    arp_pkt->proto_type     = ETHERTYPE_IPV4;
    arp_pkt->hardw_addr_len = 6;
    arp_pkt->proto_addr_len = 4;
    arp_pkt->operation      = ARP_OP_REQUEST;
    arp_pkt->sender_mac     = *my_mac;
    arp_pkt->sender_ip      = *my_ip;
    arp_pkt->target_mac     = MAC_ADDR_NULL;
    arp_pkt->target_ip      = *for_ip;
}

/**
 * arp_make_reply_packet - Creates an ARP reply packet
 * @arp_pkt: A structure to receive the packet data
 * @for_pkt: The ARP packet to which we are replying
 * @my_ip: Our own IP
 * @my_mac: Our own MAC address
 * 
 * Note that arp_pkt and for_pkt may coincide.
 */
void arp_make_reply_packet(arp_PacketV4 *arp_pkt, arp_PacketV4 *for_pkt,
    const ip_addr_t *my_ip, const mac_addr_t *my_mac)
{
    memmove(arp_pkt, for_pkt, sizeof(arp_PacketV4));
    arp_pkt->operation  = ARP_OP_REPLY;
    arp_pkt->target_mac = for_pkt->sender_mac;
    arp_pkt->target_ip  = for_pkt->sender_ip;
    arp_pkt->sender_mac = *my_mac;
    arp_pkt->sender_ip  = *my_ip;
}
