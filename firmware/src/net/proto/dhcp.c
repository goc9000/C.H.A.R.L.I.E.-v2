/*
 * dhcp.c - DHCP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <string.h>
#include <stdlib.h>

#include "dhcp.h"

/**
 * dhcp_make_packet - Creates a DHCP packet
 * @packet: A structure to hold the created packet
 * @header: The DHCP header
 * @dhcp_type: A DHCP_PKT_* constant describing the DHCP packet type
 * 
 * Note that the packet remains unclosed, i.e. it is missing an END
 * option. This will be automatically added before transmission.
 */
void dhcp_make_packet(PacketBuf* packet, dhcp_Header* header, uint8_t dhcp_type)
{
    uint8_t buf[16];
    uint8_t i;
    
    pktbuf_create(packet, STD_HEADROOM);
    
    endian_convert(header, EC_SIG_DHCP);
    pktbuf_add_data(packet, header, sizeof(dhcp_Header));
    endian_convert(header, EC_SIG_DHCP);
    
    // padding for the hardware address
    memset(buf, 0, 16);
    pktbuf_add_data(packet, buf, 16 - sizeof(mac_addr_t));
    
    // the "sname" and "file" fields
    for (i = 0; i < 12; i++) {
        pktbuf_add_data(packet, buf, 16);
    }
    
    // DHCP marker and packet type option
    buf[0] = 99;
    buf[1] = 130;
    buf[2] = 83;
    buf[3] = 99;
    pktbuf_add_data(packet, buf, 4);
    
    dhcp_add_option(packet, DHCP_OPTION_PKT_TYPE, 1, &dhcp_type);
}

/**
 * dhcp_make_discover_packet - Creates a DHCP discovery packet
 * @packet: A structure to hold the created packet
 * @my_mac: The client's MAC address
 * 
 * Returns the transaction ID.
 */
uint32_t dhcp_make_discover_packet(PacketBuf *packet, const mac_addr_t *my_mac)
{
    uint32_t xid = rand();
    dhcp_Header header;
    memset(&header, 0, sizeof(dhcp_Header));
    header.opcode = DHCP_OP_REQUEST;
    header.hardw_type = DHCP_HW_TYPE_ETHERNET;
    header.hardw_addr_len = 6;
    header.transaction_id = xid;
    header.client_hw_addr = *my_mac;
    
    dhcp_make_packet(packet, &header, DHCP_PKT_DISCOVER);
    
    return xid;
}

/**
 * dhcp_make_accept_packet - Creates a DHCP offer acceptance packet
 * @packet: A structure to hold the created packet
 * @offer_header: The corresponding offer packet's header
 * @server_id: The server ID in the offer packet
 */
void dhcp_make_accept_packet(PacketBuf *packet, const dhcp_Header *offer_header,
    const ip_addr_t *server_id)
{
    dhcp_Header header;
    memcpy(&header, offer_header, sizeof(dhcp_Header));
    
    header.opcode = DHCP_OP_REQUEST;
    header.client_ip = offer_header->your_ip;
    header.your_ip = IP_ADDR_NULL;
    header.next_server_ip = IP_ADDR_NULL;
    header.relay_ip = IP_ADDR_NULL;

    dhcp_make_packet(packet, &header, DHCP_PKT_REQUEST);
    dhcp_add_option(packet, DHCP_OPTION_SERVER_ID, 4, server_id);
}

/**
 * dhcp_make_renew_packet - Creates a DHCP renewal packet
 * @packet: A structure to hold the created packet
 * @my_mac: The client's MAC address
 * @my_ip: The client's IP address
 * 
 * Returns the transaction ID.
 */
uint32_t dhcp_make_renew_packet(PacketBuf *packet,
    const mac_addr_t *my_mac, const ip_addr_t *my_ip)
{
    uint32_t xid = rand();
    dhcp_Header header;
    memset(&header, 0, sizeof(dhcp_Header));
    header.opcode = DHCP_OP_REQUEST;
    header.hardw_type = DHCP_HW_TYPE_ETHERNET;
    header.hardw_addr_len = 6;
    header.transaction_id = xid;
    header.client_hw_addr = *my_mac;
    header.client_ip = *my_ip;
    
    dhcp_make_packet(packet, &header, DHCP_PKT_REQUEST);

    return xid;
}

/**
 * dhcp_add_option - Adds an option to a DHCP packet
 * @packet: A packet buffer holding the DHCP packet
 * @option: A DHCP_OPTION_* constant identifying the option type
 * @length: The length of the option value, in bytes
 * @data: A buffer holding the option value
 */
void dhcp_add_option(PacketBuf *packet, uint8_t option, uint8_t length,
    const void *data)
{
    pktbuf_add_data(packet, &option, 1);
    
    if ((option != DHCP_OPTION_PAD) && (option != DHCP_OPTION_END)) {
        pktbuf_add_data(packet, &length, 1);
        pktbuf_add_data(packet, data, length);
    }
}

/**
 * dhcp_parse_options - Parses the options in a DHCP packet
 * @packet: A packet buffer containing only the DHCP options
 * @pkt_type: A variable to store the packet type
 * @server_id: A variable to store the server ID
 * @gateway: A variable to store the gateway IP
 * @net_mask: A variable to store the netmask
 * @renew_time: A variable to store the renewal time
 */
void dhcp_parse_options(PacketBuf *packet, uint8_t *pkt_type,
    ip_addr_t *server_id, ip_addr_t *gateway, ip_addr_t *net_mask,
    uint32_t *renew_time)
{
    uint8_t opt, len;
    while (!pktbuf_empty(packet)) {
        pktbuf_get_header(packet, &opt, 1, 0);
        
        if (opt == DHCP_OPTION_PAD) {
            continue;
        }
        if (opt == DHCP_OPTION_END) {
            break;
        }
        
        pktbuf_get_header(packet, &len, 1, 0);
        
        switch (opt) {
            case DHCP_OPTION_SERVER_ID:
                pktbuf_get_header(packet, server_id, 4, 0);
                break;
            case DHCP_OPTION_GATEWAY:
                pktbuf_get_header(packet, gateway, 4, 0);
                break;
            case DHCP_OPTION_NETMASK:
                pktbuf_get_header(packet, net_mask, 4, 0);
                break;
            case DHCP_OPTION_RENEW_TIME:
                pktbuf_get_header(packet, renew_time, 4, 0);
                break;
            case DHCP_OPTION_PKT_TYPE:
                pktbuf_get_header(packet, pkt_type, 1, 0);
                break;
            default:
                pktbuf_pull(packet, len);
                break;
        }
    }
}
