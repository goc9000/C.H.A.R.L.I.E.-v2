/*
 * net_core.c - Core network functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "debug/debug.h"
#include "core/sched.h"
#include "dev/enc28j60.h"
#include "arp_impl.h"
#include "eth_impl.h"
#include "dhcp_impl.h"
#include "tcp_impl.h"
#include "packet_buf.h"
#include "net_core.h"

static struct {
    bool ip_configured:1;
    struct {
        ip_addr_t ip;
        uint8_t net_mask;
        ip_addr_t gateway_ip;
    } cfg;
} net;

void _net_post_linkup_init(uint32_t dummy __attribute__((unused)))
{
    if (!enc28j60_check_link_up()) {
        sched_schedule(CFG_NET_RECEIVE_INTERVAL, _net_post_linkup_init);
        return;
    }
    
    sched_schedule(SCHED_SEC/2, dhcp_start_client);
}

/**
 * net_set_config - Sets the IP config of the network
 * @ip: The new IP address
 * @net_mask: The new netmask
 * @gateway_ip: The new gateway address
 */
void net_set_config(const ip_addr_t *ip, uint8_t net_mask,
    const ip_addr_t *gateway_ip)
{
    net.ip_configured = !ip_addr_equal(ip, &IP_ADDR_NULL);
    net.cfg.ip = *ip;
    net.cfg.net_mask = net_mask;
    net.cfg.gateway_ip = *gateway_ip;
}

/**
 * net_get_config - Gets parameters in the current network config
 * @mac_addr: A variable to store the current MAC. May be NULL
 * @ip_addr: A variable to store the current IP. May be NULL
 * @net_mask: A variable to store the netmask. May be NULL
 * @gw_addr: A variable to store the gateway IP. May be NULL
 */
void net_get_config(mac_addr_t *mac_addr, ip_addr_t *ip_addr,
    uint8_t *net_mask, ip_addr_t *gw_addr)
{
    if (mac_addr) {
        enc28j60_get_mac_addr(mac_addr);
    }
    if (ip_addr) {
        *ip_addr = net.cfg.ip;
    }
    if (net_mask) {
        *net_mask = net.cfg.net_mask;
    }
    if (gw_addr) {
        *gw_addr = net.cfg.gateway_ip;
    }
}

/**
 * net_is_for_my_ip - Checks whether an IP address targets us
 * @ip_addr: The IP address to test
 */
bool net_is_for_my_ip(const ip_addr_t *ip_addr)
{
    return ip_addr_match(ip_addr, &net.cfg.ip, net.cfg.net_mask) ||
        ip_addr_equal(&net.cfg.ip, &IP_ADDR_NULL);
}

/**
 * net_is_broadcast_ip - Checks for a local or global broadcast IP
 * @ip_addr: The IP address to test
 */
bool net_is_broadcast_ip(const ip_addr_t *ip_addr)
{
    return ip_is_broadcast_ip(ip_addr, net.cfg.net_mask);
}

/**
 * net_is_in_local_net - Checks for an IP address in our network
 * @ip_addr: The IP address to test
 */
bool net_is_in_local_net(const ip_addr_t *ip_addr)
{
    return ip_same_network(ip_addr, &net.cfg.ip, net.cfg.net_mask);
}

/**
 * net_receive - Checks for received packets and reacts to them
 */
void net_receive(uint32_t dummy __attribute__((unused)))
{
    PacketBuf pkt;
    uint32_t status;
    
    while (enc28j60_get_pending_frames() > 0) {
        enc28j60_get_received_frame(&pkt, &status);
        eth_receive(&pkt);
        enc28j60_free_received_frame();
    }
    
    sched_schedule(CFG_NET_RECEIVE_INTERVAL, net_receive);
}

/**
 * net_busy - Checks whether we are busy with some important action
 *            in the network module
 */
bool net_busy(void)
{
    return tcp_busy();
}

/**
 * net_init - Initializes network stack
 */
void net_init(void)
{
    net_set_config(&IP_ADDR_NULL, 0, &IP_ADDR_NULL);
    arp_init();
    dhcp_init();
    tcp_init();
    
    sched_schedule(0, net_receive);
    sched_schedule(CFG_NET_RECEIVE_INTERVAL, _net_post_linkup_init);
}
