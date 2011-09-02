/*
 * arp_impl.c - ARP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <stdlib.h>

#include "debug/debug.h"
#include "arp_impl.h"
#include "eth_impl.h"
#include "net_core.h"
#include "dot_config.h"

static struct {
    arp_TableEntry table[CFG_NET_ARP_TABLE_ENTRIES];
    uint8_t table_ptr;
} arp;

/**
 * arp_receive - Handles the receipt of an ARP packet
 * @packet: A packet buffer containing the packet
 */
void arp_receive(PacketBuf *packet)
{
    arp_PacketV4 arp_pkt;
    mac_addr_t my_mac;
    ip_addr_t my_ip;
    
    pktbuf_get_header(packet, &arp_pkt, sizeof(arp_PacketV4), EC_SIG_ARPV4);
    
    if (!(
        (arp_pkt.hardw_type == ARP_HW_TYPE_ETHERNET) &&
        (arp_pkt.proto_type == ETHERTYPE_IPV4) &&
        (arp_pkt.hardw_addr_len == 6) &&
        (arp_pkt.proto_addr_len == 4) &&
        net_is_for_my_ip(&(arp_pkt.target_ip)))) {
        return;
    }

    // Reply to queries
    if (arp_pkt.operation == ARP_OP_REQUEST) {
        net_get_config(&my_mac, &my_ip, NULL, NULL);
        if (ip_addr_equal(&my_ip, &IP_ADDR_NULL)) {
            return;
        }
        arp_make_reply_packet(&arp_pkt, &arp_pkt, &my_ip, &my_mac);
        arp_transmit(&arp_pkt);
        return;
    }
    
    // Receive answers to our queries
    if (arp_pkt.operation == ARP_OP_REPLY) {
        arp_learn(&arp_pkt.sender_ip, &arp_pkt.sender_mac);
    }
}

/**
 * arp_send - Sends an ARP packet
 * @arp_pkt: A structure containing ARP packet data
 */
void arp_transmit(arp_PacketV4 *arp_pkt)
{
    mac_addr_t *dest_mac = (arp_pkt->operation == ARP_OP_REQUEST) ?
        &MAC_ADDR_BCAST : &(arp_pkt->target_mac);
    PacketBuf packet;
    
    pktbuf_create_in(&packet, PKTBUF_AREA_OOB, STD_HEADROOM);
    pktbuf_add_header(&packet, arp_pkt, sizeof(arp_PacketV4), EC_SIG_ARPV4);
    eth_send(&packet, dest_mac, ETHERTYPE_ARP);
}

/**
 * arp_learn - Learns a new (IP,MAC) association
 * @ip_addr: An IP address
 * @mac_addr: A MAC address
 */
void arp_learn(const ip_addr_t *ip_addr, const mac_addr_t *mac_addr)
{
    uint8_t i, ptr;
    
    for (i=0; i < CFG_NET_ARP_TABLE_ENTRIES; i++) {
        if (ip_addr_equal(ip_addr, &(arp.table[i].ip_addr))) {
            arp.table[i].mac_addr = *mac_addr;
            return;
        }
    }
    
    ptr = arp.table_ptr;
    arp.table[ptr].ip_addr = *ip_addr;
    arp.table[ptr].mac_addr = *mac_addr;
    ++ptr;
    if (ptr >= CFG_NET_ARP_TABLE_ENTRIES) {
        ptr = 0;
    }

    arp.table_ptr = ptr;
}

/**
 * arp_lookup - Looks up the MAC address for an IP.
 * 
 * Note: If the address is not found, the broadcast MAC will be used
 */
bool arp_lookup(const ip_addr_t *ip_addr, mac_addr_t *mac_addr)
{
    uint8_t i;
    ip_addr_t search_ip = *ip_addr;
    *mac_addr = MAC_ADDR_BCAST;
        
    if (net_is_broadcast_ip(&search_ip)) {
        return TRUE;
    }
    
    // for non-local-net addresses, use the gateway
    if (!net_is_in_local_net(&search_ip)) {
        net_get_config(NULL, NULL, NULL, &search_ip);
    }
    
    for (i = 0; i < CFG_NET_ARP_TABLE_ENTRIES; i++) {
        if (ip_addr_equal(&search_ip, &(arp.table[i].ip_addr))) {
            *mac_addr = arp.table[i].mac_addr;
            return TRUE;
        }
    }
    
    return FALSE;
}

/**
 * arp_init - Initializes the ARP table
 */
void arp_init(void)
{
    uint8_t i;
    for (i = 0; i < CFG_NET_ARP_TABLE_ENTRIES; i++)
    {
        arp.table[i].ip_addr = IP_ADDR_BCAST;
        arp.table[i].mac_addr = MAC_ADDR_BCAST;
    }

    arp.table_ptr = 0;
}
