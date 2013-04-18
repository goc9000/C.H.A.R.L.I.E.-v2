/*
 * dhcp_impl.c - DHCP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */
 
#include <string.h>

#include "debug/debug.h"
#include "net_core.h"
#include "dhcp_impl.h"
#include "udp_impl.h"
#include "core/sched.h"
#include "dot_config.h"

#define RENEW_TIME_INF      0xFFFFFFFF

#define STATUS_FINISHED     0
#define STATUS_RECV_OFFERS  1
#define STATUS_WAIT_ACK     2

static struct {
    uint8_t status;
    uint32_t transaction_id;
    ip_addr_t issuing_server;
} dhcp;

void dhcp_renew_address(uint32_t dummy __attribute__((unused)));

/**
 * dhcp_receive - Handles the receipt of a DHCP packet
 * @packet: A packet buffer containing the packet
 * @ip_hdr: The extracted IP header
 */
void dhcp_receive(PacketBuf *packet, ip_Header *ip_hdr)
{
    dhcp_Header header;
    uint8_t pkt_type = 0;
    ip_addr_t server_id = IP_ADDR_NULL;
    ip_addr_t gateway = IP_ADDR_NULL;
    ip_addr_t net_mask = IP_ADDR_NULL;
    uint32_t renew_time = 0;
    
    pktbuf_get_header(packet, &header, sizeof(dhcp_Header), EC_SIG_DHCP);
    // ignore padding, sname, file, DHCP sig
    pktbuf_pull(packet, 10+64+128+4);
    
    dhcp_parse_options(packet, &pkt_type, &server_id, &gateway,
        &net_mask, &renew_time);
    
    if (header.transaction_id != dhcp.transaction_id) {
        return;
    }
    
    if ((pkt_type == DHCP_PKT_OFFER) && (dhcp.status == STATUS_RECV_OFFERS)) {
        sched_unschedule(dhcp_start_client);
        
        dhcp_make_accept_packet(packet, &header, &server_id);
        dhcp_send(packet, &header.next_server_ip);
        
        dhcp.status = STATUS_WAIT_ACK;
        sched_schedule(SCHED_SEC * CFG_NET_DHCP_TIMEOUT_SEC,
            dhcp_start_client);
    }

    if ((pkt_type == DHCP_PKT_ACK) && (dhcp.status == STATUS_WAIT_ACK)) {
        sched_unschedule(dhcp_start_client);
        sched_unschedule(dhcp_renew_address);
        
        net_set_config(&(header.your_ip), ip_decode_net_mask(&net_mask),
            &gateway);
        
        dhcp.issuing_server = ip_hdr->source_ip;
        dhcp.status = STATUS_FINISHED;
        
        // renew
        if (renew_time != RENEW_TIME_INF) {
            sched_schedule_param(SCHED_SEC * renew_time, dhcp_renew_address, 3);
        }
    }
}

/**
 * dhcp_send - Sends a DHCP packet
 * @packet: The DHCP packet to send
 * @dest_ip: The destination IP
 */
void dhcp_send(PacketBuf *packet, const ip_addr_t *dest_ip)
{
    dhcp_add_option(packet, DHCP_OPTION_END, 0, NULL);
    udp_send(packet, dest_ip, DHCP_CLIENT_PORT, DHCP_SERVER_PORT);
    pktbuf_retract(packet, 1);
}

/**
 * dhcp_renew_address - Callback for renewing the lease
 * @tries: Tries left
 */
void dhcp_renew_address(uint32_t tries)
{
    PacketBuf packet;
    mac_addr_t my_mac;
    ip_addr_t my_ip;
    
    if (!tries) {
        dhcp_start_client(0);
        return;
    }
    
    net_get_config(&my_mac, &my_ip, NULL, NULL);
    
    dhcp.transaction_id = dhcp_make_renew_packet(&packet, &my_mac, &my_ip);
    dhcp_send(&packet, &dhcp.issuing_server);
    dhcp.status = STATUS_WAIT_ACK;
    
    sched_schedule_param(SCHED_SEC * CFG_NET_DHCP_TIMEOUT_SEC,
        dhcp_renew_address, tries - 1);
}

/**
 * dhcp_start_client - Initiates a DHCP client transaction
 */
void dhcp_start_client(uint32_t dummy __attribute__((unused)))
{
    PacketBuf packet;
    mac_addr_t my_mac;
    
    net_get_config(&my_mac, NULL, NULL, NULL);
    
    dhcp.status = STATUS_RECV_OFFERS;
    dhcp.transaction_id = dhcp_make_discover_packet(&packet, &my_mac);
    dhcp_send(&packet, &IP_ADDR_BCAST);
    
    sched_schedule(SCHED_SEC * CFG_NET_DHCP_TIMEOUT_SEC,
        dhcp_start_client);
}

/**
 * dhcp_init - Initializes the DHCP client
 */
void dhcp_init(void)
{
    dhcp.status = STATUS_FINISHED;
}
