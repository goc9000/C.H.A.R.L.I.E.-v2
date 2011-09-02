/*
 * tcp.c - TCP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "dot_config.h"
#include "tcp.h"

typedef struct {
    ip_addr_t source_ip;
    ip_addr_t dest_ip;
    uint8_t zero;
    uint8_t protocol;
    uint16_t tcp_length;
} tcp_PseudoIpHeader;

/**
 * tcp_is_connection_begin_seg - Checks for a connection start segment
 * @header: A segment's header
 */
bool tcp_is_connection_begin_seg(const tcp_Header *header)
{
    return (header->flags & TCP_FLAG_SYN) && !(header->flags & TCP_FLAG_ACK);
}

/**
 * tcp_get_header - Extract TCP header from packet
 * @packet: A packet buffer containing a TCP segment
 * @tcp_hdr: A structure to store the extracted header
 */
void tcp_get_header(PacketBuf *packet, tcp_Header *tcp_hdr)
{
    uint8_t hdr_size;
    
    pktbuf_get_header(packet, tcp_hdr, sizeof(tcp_Header), EC_SIG_TCP);
    
    hdr_size = ((uint8_t)tcp_hdr->ihl) << 2;
    pktbuf_pull(packet, hdr_size - sizeof(tcp_Header));
}

/**
 * tcp_update_hdr_checksum - Updates the checksum in a TCP header
 * @header: A TCP header
 * @payload: A packet buffer containing the TCP payload
 * @source_ip: The source IP
 * @dest_ip: The destination IP
 * 
 * Note: the header must be stored using host endianness.
 */
void tcp_update_hdr_checksum(tcp_Header *header, PacketBuf *payload,
    const ip_addr_t *source_ip, const ip_addr_t *dest_ip)
{
    tcp_PseudoIpHeader pseudo_ip;
    
    pseudo_ip.source_ip = *source_ip;
    pseudo_ip.dest_ip = *dest_ip;
    pseudo_ip.zero = 0;
    pseudo_ip.protocol = IP_PROTO_TCP;
    pseudo_ip.tcp_length = htons(sizeof(tcp_Header) + payload->length);
    
    endian_convert(header, EC_SIG_TCP);
    header->checksum = 0;
    header->checksum = ip_join_checksums(
        ip_checksum(&pseudo_ip, sizeof(tcp_PseudoIpHeader)),
        ip_join_checksums(
            ip_checksum(header, sizeof(tcp_Header)),
            pktbuf_ip_checksum(payload)
        )
    );
    
    endian_convert(header, EC_SIG_TCP);
}

/**
 * tcp_make_reply_header - Creates the header for a TCP reply packet
 * @header: A structure to hold the created header
 * @request: The header for the segment we are replying to
 * @seq_no: The sequence number to use
 * @ack_no: The acknowledgement number to use
 * @flags: The TCP flags to set
 * 
 * Note: header and request may be the same.
 */
void tcp_make_reply_header(tcp_Header *header, tcp_Header *request,
    uint32_t seq_no, uint32_t ack_no, uint8_t flags)
{
    uint16_t src_port = request->source_port;
    
    header->source_port = request->dest_port;
    header->dest_port = src_port;
    header->seq_no = seq_no;
    header->ack_no = ack_no;
    header->reserved = 0;
    header->ihl = 5;
    header->flags = flags;
    header->window_size = CFG_NET_TCP_WINDOW_SIZE;
    header->checksum = 0;
    header->urgent_ptr = 0;
}
