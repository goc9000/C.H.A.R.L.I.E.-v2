/*
 * tcp_impl.c - TCP layer implementation
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <stdlib.h>
#include <string.h>

#include "debug/debug.h"
#include "core/sched.h"
#include "time/time.h"
#include "net_core.h"
#include "ip_impl.h"
#include "tcp_impl.h"
#include "web_impl.h"
#include "dot_config.h"

// State flags
#define SF_EXISTS       1
#define SF_RECEIVING    2
#define SF_RESPONDING   4
#define SF_MY_FIN       8
#define SF_MY_FIN_ACKED 16
#define SF_ABORTING     32
#define SF_CLIENT_FIN   64

static struct {
    struct {
        uint8_t flags;
        ip_addr_t peer_ip;
        uint16_t  peer_port;
        uint32_t  peer_seq;
        uint16_t  my_port;
        uint32_t my_seq;
        uint8_t failures;
        PacketBuf xmit_buffer;
    } current;

    bool active;
} tcp;

static void _tcp_timeout(uint32_t dummy __attribute__((unused)));

static void _tcp_activity_lapse(uint32_t dummy __attribute__((unused)))
{
    tcp.active = FALSE;
}

static void _tcp_activity(void)
{
    tcp.active = TRUE;
    sched_schedule(SCHED_SEC * CFG_NET_TCP_LAPSE_SEC, _tcp_activity_lapse);
}

static void _tcp_start_timeout(void)
{
    sched_schedule(SCHED_SEC * CFG_NET_TCP_TIMEOUT_SEC, _tcp_timeout);
}

static void _tcp_stop_timeout(void)
{
    sched_unschedule(_tcp_timeout);
}

static void _tcp_reset_xmit_buffer(void)
{
    tcp.current.xmit_buffer.start = CFG_E28J_AUX_BUFFER_START;
    tcp.current.xmit_buffer.length = 0;
}

static bool _tcp_xmit_buffered(void)
{
    return (tcp.current.xmit_buffer.length > 0);
}

static bool _tcp_pending_my_fin(void)
{
    return (tcp.current.flags & SF_MY_FIN) &&
        !(tcp.current.flags & SF_MY_FIN_ACKED);
}

static bool _tcp_connection_active(void)
{
    return (tcp.current.flags & SF_EXISTS) != 0;
}

static void _tcp_accept_connection(tcp_Header* tcp_header,
    const ip_Header* ip_header)
{
    tcp_Header reply_header;
    PacketBuf packet;

    _tcp_activity();
    
    tcp_make_reply_header(&reply_header, tcp_header, 1,
        tcp_header->seq_no + 1, TCP_FLAG_SYN | TCP_FLAG_ACK);
    pktbuf_create(&packet, STD_HEADROOM);
    tcp_send(&packet, &reply_header, &ip_header->source_ip);
}

static void _tcp_reply_on_pending_connection(tcp_Header *tcp_header,
    const ip_Header *ip_hdr, bool empty_seg)
{
    tcp_Header reply_header;
    PacketBuf packet;

    _tcp_activity();
    
    tcp_make_reply_header(&reply_header, tcp_header, 2,
        tcp_header->seq_no, TCP_FLAG_ACK);
    
    if (tcp_header->flags & TCP_FLAG_FIN) {
        reply_header.flags |= TCP_FLAG_FIN;
        reply_header.ack_no++;
    }
    
    if (!empty_seg) {
        reply_header.window_size = 0;
    }
    
    pktbuf_create(&packet, STD_HEADROOM);
    tcp_send(&packet, &reply_header, &ip_hdr->source_ip);
}

static void _tcp_activate_connection(const tcp_Header *tcp_header,
    const ip_Header *ip_hdr)
{
    tcp.current.flags = SF_EXISTS;
    tcp.current.my_seq = 2;
    tcp.current.peer_ip   = ip_hdr->source_ip;
    tcp.current.peer_port = tcp_header->source_port;
    tcp.current.peer_seq  = tcp_header->seq_no;
    tcp.current.my_port   = tcp_header->dest_port;
    tcp.current.failures = 0;
    _tcp_reset_xmit_buffer();
}

static bool _tcp_is_seg_for_active_connection(const tcp_Header *tcp_header,
    const ip_Header *ip_hdr)
{
    return
        _tcp_connection_active() &&
        ip_addr_equal(&ip_hdr->source_ip, &tcp.current.peer_ip) &&
        (tcp_header->source_port == tcp.current.peer_port) &&
        (tcp_header->dest_port == tcp.current.my_port);
}

static void _tcp_reply_on_current_connection(void)
{
    PacketBuf packet;
    tcp_Header header;
    
    header.source_port = tcp.current.my_port;
    header.dest_port = tcp.current.peer_port;
    header.seq_no = tcp.current.my_seq;
    header.ack_no = tcp.current.peer_seq;
    header.reserved = 0;
    header.ihl = 5;
    header.flags = TCP_FLAG_ACK;
    header.window_size = CFG_NET_TCP_WINDOW_SIZE;
    header.checksum = 0;
    header.urgent_ptr = 0;

    if (_tcp_pending_my_fin()) {
        header.flags |= TCP_FLAG_FIN;
    }
    if (tcp.current.flags & SF_ABORTING) {
        header.flags = TCP_FLAG_RST;
    }
    
    pktbuf_create(&packet, STD_HEADROOM);
    pktbuf_add_buf(&packet, &tcp.current.xmit_buffer);

    tcp_send(&packet, &header, &tcp.current.peer_ip);
    
    _tcp_activity();
    _tcp_start_timeout();
}

static void _tcp_end_connection(void)
{
    if (!_tcp_connection_active()) {
        return;
    }
    
    _tcp_stop_timeout();

    tcp.current.flags &= ~SF_EXISTS;
}

static void _tcp_abort_connection(void)
{
    if (!_tcp_connection_active()) {
        return;
    }
        
    tcp.current.flags |= SF_ABORTING;
    _tcp_reset_xmit_buffer();
    _tcp_reply_on_current_connection();
    _tcp_end_connection();
}

static void _tcp_timeout(uint32_t dummy __attribute__((unused)))
{
    if (!_tcp_connection_active()) {
        return;
    }
    
    tcp.current.failures++;

    if (tcp.current.failures < CFG_NET_TCP_MAX_FAILURES) {
        _tcp_reply_on_current_connection();
    } else {
        _tcp_abort_connection();
    }
}

static void _tcp_receive_for_current(tcp_Header* tcp_header,
    PacketBuf *payload)
{
    uint32_t expected_ack = tcp.current.my_seq +
        tcp.current.xmit_buffer.length + _tcp_pending_my_fin();
    bool has_payload = !!(payload->length);
    bool is_finish = !!(tcp_header->flags & TCP_FLAG_FIN);
    bool has_correct_seq = (tcp_header->seq_no == tcp.current.peer_seq);
    bool has_correct_ack = (tcp_header->ack_no == expected_ack);
    bool should_respond = has_payload || is_finish || !has_correct_seq;
    
    _tcp_activity();
    _tcp_start_timeout();
    
    if (has_correct_seq) {
        tcp.current.peer_seq += payload->length;
        if (is_finish &&
            !(tcp.current.flags & SF_CLIENT_FIN)) {
            tcp.current.flags |= SF_CLIENT_FIN;
            tcp.current.peer_seq++;
        }

        if (has_payload) {
            if (!(tcp.current.flags & SF_RECEIVING)) {
                tcp_start_request(tcp.current.my_port);
                tcp.current.flags |= SF_RECEIVING;
            }
            if (!tcp_receive_request(payload, tcp.current.my_port)) {
                tcp.current.flags |= SF_RESPONDING;
            }
        }
    }

    if (has_correct_ack) {
        tcp.current.my_seq = expected_ack;
        _tcp_reset_xmit_buffer();
        
        if (tcp.current.flags & SF_MY_FIN) {
            tcp.current.flags |= SF_MY_FIN_ACKED;
        }

        if ((tcp.current.flags & SF_RESPONDING) && !(tcp.current.flags & SF_MY_FIN)) {
            if (!tcp_get_answer(&tcp.current.xmit_buffer, tcp.current.my_port)) {
                tcp.current.flags |= SF_MY_FIN;
            }
        }

        if (_tcp_xmit_buffered() || _tcp_pending_my_fin()) {
            should_respond = TRUE;
        }
    }

    if (tcp_header->flags & TCP_FLAG_RST) {
        tcp.current.peer_seq++;
        _tcp_reset_xmit_buffer();
        _tcp_reply_on_current_connection();
        _tcp_end_connection();
        return;
    }

    tcp.current.failures = (has_correct_seq && has_correct_ack) 
        ? 0 : tcp.current.failures + 1;
    if (should_respond) {
        _tcp_reply_on_current_connection();
    }

    if ((tcp.current.flags & SF_CLIENT_FIN) &&
       (tcp.current.flags & SF_MY_FIN_ACKED)) {
        _tcp_end_connection();
    }
}

/**
 * tcp_receive - Handles the receipt of a TCP segment
 * @packet: A packet buffer containing the segment
 * @ip_hdr: The extracted IP header
 */
void tcp_receive(PacketBuf *packet, const ip_Header *ip_hdr)
{
    tcp_Header header;
    
    tcp_get_header(packet, &header);
    
    if (tcp_is_connection_begin_seg(&header)) {
        if (tcp_port_is_open(header.dest_port)) {
            _tcp_accept_connection(&header, ip_hdr);
        }
    } else if (header.ack_no == 2) {
        if (!_tcp_connection_active() && (packet->length > 0)) {
            _tcp_activate_connection(&header, ip_hdr);
            _tcp_receive_for_current(&header, packet);
        } else {
            _tcp_reply_on_pending_connection(&header, ip_hdr, !packet->length);
        }
    } else if (_tcp_is_seg_for_active_connection(&header, ip_hdr)) {
        _tcp_receive_for_current(&header, packet);
    }
}

/**
 * tcp_send - Sends a TCP segment
 * @packet: A packet buffer holding the segment payload
 * @tcp_hdr: The header for the segment
 * @dest_ip: The destination IP address
 * 
 * Note that this only handles the actual transmission of the packet.
 * Retransmission, etc. must be handled by the caller.
 */
void tcp_send(PacketBuf *packet, tcp_Header *tcp_hdr, const ip_addr_t *dest_ip)
{
    ip_addr_t my_ip;
    
    net_get_config(NULL, &my_ip, NULL, NULL);
    
    tcp_update_hdr_checksum(tcp_hdr, packet, &my_ip, dest_ip);
    
    pktbuf_add_header(packet, tcp_hdr, sizeof(tcp_Header), EC_SIG_TCP);
    ip_send(packet, dest_ip, IP_PROTO_TCP);
    pktbuf_pull(packet, sizeof(tcp_Header));
}

/**
 * tcp_port_is_open - Checks whether we are accepting connections on a
 *                    given TCP port
 * @port: A TCP port
 */
bool tcp_port_is_open(uint16_t port)
{
    return (port == CFG_NET_TCP_PORT_HTTP);
}

/**
 * tcp_start_request - Starts reading a request on a given TCP port
 * @port: A TCP port
 */
void tcp_start_request(uint16_t port)
{
    if (port == CFG_NET_TCP_PORT_HTTP) {
        web_new_request();
    }
}

/**
 * tcp_receive_request - Receives more request data
 * @data: A packet buffer containing the received data
 * @port: A TCP port
 * 
 * Returns FALSE once the request has been fully read and a response
 * can be returned.
 */
bool tcp_receive_request(PacketBuf* data, uint16_t port)
{
    if (port == CFG_NET_TCP_PORT_HTTP) {
        return web_receive_request(data);
    }
    
    return FALSE;
}

/**
 * tcp_get_answer - Retrieves more answer data
 * @data: A packet buffer to be filled with a response data chunk
 * @port: A TCP port
 * 
 * Returns FALSE once the last answer bit thas been extracted.
 */
bool tcp_get_answer(PacketBuf* data, uint16_t port)
{
    if (port == CFG_NET_TCP_PORT_HTTP) {
        return web_get_answer(data);
    }
    
    return FALSE;
}

/**
 * tcp_busy - Checks whether the TCP stack is busy
 */
bool tcp_busy(void)
{
    return tcp.active;
}

/**
 * tcp_init - Initializes the TCP subsystem
 */
void tcp_init(void)
{
    tcp.active = FALSE;
    tcp.current.flags = 0;
    _tcp_reset_xmit_buffer();
}
