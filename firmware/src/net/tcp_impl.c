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

typedef struct {
    ip_addr_t peer_ip;
    uint16_t  peer_port;
    uint32_t  peer_seq;
    uint16_t  my_port;
} tcp_ConnectionInfo;

static struct {
    struct {
        uint8_t flags;
        tcp_ConnectionInfo conn;
        uint32_t my_seq;
        uint8_t failures;
        PacketBuf xmit_buffer;
    } current;

    tcp_ConnectionInfo pending[CFG_NET_TCP_MAX_PENDING];
    uint8_t num_pending;
    bool active;
} tcp;

void _tcp_timeout(uint32_t dummy __attribute__((unused)));
static void _tcp_activate_next_connection(void);

void _tcp_activity_lapse(uint32_t dummy __attribute__((unused)))
{
    tcp.active = FALSE;
}

void _tcp_activity(void)
{
    tcp.active = TRUE;
    sched_schedule(SCHED_SEC * CFG_NET_TCP_LAPSE_SEC, _tcp_activity_lapse);
}

void _tcp_start_timeout(void)
{
    sched_schedule(SCHED_SEC * CFG_NET_TCP_TIMEOUT_SEC, _tcp_timeout);
}

void _tcp_stop_timeout(void)
{
    sched_unschedule(_tcp_timeout);
}

void _tcp_reset_xmit_buffer(void)
{
    tcp.current.xmit_buffer.start = CFG_E28J_AUX_BUFFER_START;
    tcp.current.xmit_buffer.length = 0;
}

bool _tcp_xmit_buffered(void)
{
    return (tcp.current.xmit_buffer.length > 0);
}

bool _tcp_pending_my_fin(void)
{
    return (tcp.current.flags & SF_MY_FIN) &&
        !(tcp.current.flags & SF_MY_FIN_ACKED);
}

static void _tcp_accept_connection(tcp_Header* tcp_header, const tcp_ConnectionInfo *conn_info)
{
    tcp_Header reply_header;
    PacketBuf packet;

    _tcp_activity();

    tcp_make_reply_header(&reply_header, tcp_header, 1,
        tcp_header->seq_no + 1, TCP_FLAG_SYN | TCP_FLAG_ACK);
    
    if (tcp.current.flags & SF_EXISTS) {
        // Already handling connection, try to accept as pending
        if (tcp.num_pending < CFG_NET_TCP_MAX_PENDING) {
            // Accept as pending
            tcp.pending[tcp.num_pending] = *conn_info;
            tcp.pending[tcp.num_pending].peer_seq++;
            tcp.num_pending++;
            reply_header.window_size = 0;
        } else {
            // Reject (out of room for pending connections)
            return;
        }
    } else {
        // Accept as current
        tcp.current.flags = SF_EXISTS;
        tcp.current.conn = *conn_info;
        tcp.current.conn.peer_seq++;
        tcp.current.my_seq = 2;
        tcp.current.failures = 0;
        _tcp_reset_xmit_buffer();
        _tcp_start_timeout();
    }

    pktbuf_create(&packet, STD_HEADROOM);
    tcp_send(&packet, &reply_header, &(conn_info->peer_ip));
}

static void _tcp_reply_on_connection(void)
{
    PacketBuf packet;
    tcp_Header header;
    
    header.source_port = tcp.current.conn.my_port;
    header.dest_port = tcp.current.conn.peer_port;
    header.seq_no = tcp.current.my_seq;
    header.ack_no = tcp.current.conn.peer_seq;
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

    tcp_send(&packet, &header, &tcp.current.conn.peer_ip);
    
    _tcp_activity();
    _tcp_start_timeout();
}

static void _tcp_end_connection(void)
{
    if (!(tcp.current.flags & SF_EXISTS)) {
        return;
    }
    
    _tcp_stop_timeout();

    tcp.current.flags &= ~SF_EXISTS;
    
    _tcp_activate_next_connection();
}

static void _tcp_abort_connection(void)
{
    if (!(tcp.current.flags & SF_EXISTS)) {
        return;
    }
        
    tcp.current.flags |= SF_ABORTING;
    _tcp_reset_xmit_buffer();
    _tcp_reply_on_connection();
    _tcp_end_connection();
}

static void _tcp_activate_next_connection(void)
{
    if (tcp.num_pending) {
        tcp.current.flags = SF_EXISTS;
        tcp.current.my_seq = 2;
        tcp.current.conn = tcp.pending[0];
        tcp.current.failures = 0;
        _tcp_reset_xmit_buffer();

        memmove(tcp.pending, tcp.pending + 1,
            sizeof(tcp_ConnectionInfo) * (tcp.num_pending - 1));
        tcp.num_pending--;

        _tcp_reply_on_connection();
    }
}

void _tcp_timeout(uint32_t dummy __attribute__((unused)))
{
    if (!(tcp.current.flags & SF_EXISTS)) {
        return;
    }
    
    tcp.current.failures++;

    if (tcp.current.failures < CFG_NET_TCP_MAX_FAILURES) {
        _tcp_reply_on_connection();
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
    bool has_correct_seq = (tcp_header->seq_no == tcp.current.conn.peer_seq);
    bool has_correct_ack = (tcp_header->ack_no == expected_ack);
    bool should_respond = has_payload || is_finish || !has_correct_seq;
    
    _tcp_activity();
    _tcp_start_timeout();
    
    if (has_correct_seq) {
        tcp.current.conn.peer_seq += payload->length;
        if (is_finish &&
            !(tcp.current.flags & SF_CLIENT_FIN)) {
            tcp.current.flags |= SF_CLIENT_FIN;
            tcp.current.conn.peer_seq++;
        }

        if (has_payload) {
            if (!(tcp.current.flags & SF_RECEIVING)) {
                tcp_start_request(tcp.current.conn.my_port);
                tcp.current.flags |= SF_RECEIVING;
            }
            if (!tcp_receive_request(payload, tcp.current.conn.my_port)) {
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
            if (!tcp_get_answer(&tcp.current.xmit_buffer, tcp.current.conn.my_port)) {
                tcp.current.flags |= SF_MY_FIN;
            }
        }

        if (_tcp_xmit_buffered() || _tcp_pending_my_fin()) {
            should_respond = TRUE;
        }
    }

    if (tcp_header->flags & TCP_FLAG_RST) {
        tcp.current.conn.peer_seq++;
        _tcp_reset_xmit_buffer();
        _tcp_reply_on_connection();
        _tcp_end_connection();
        return;
    }

    tcp.current.failures = (has_correct_seq && has_correct_ack) 
        ? 0 : tcp.current.failures + 1;
    if (should_respond) {
        _tcp_reply_on_connection();
    }

    if ((tcp.current.flags & SF_CLIENT_FIN) &&
       (tcp.current.flags & SF_MY_FIN_ACKED)) {
        _tcp_end_connection();
    }
}

static void _tcp_receive_for_pending(tcp_Header* tcp_header,
    const tcp_ConnectionInfo *conn_info)
{
    tcp_Header reply_header;
    PacketBuf packet;
    
    _tcp_activity();

    tcp_make_reply_header(&reply_header, tcp_header, 2,
        tcp_header->seq_no, TCP_FLAG_ACK);
    reply_header.window_size = 0;
    
    pktbuf_create(&packet, STD_HEADROOM);
    tcp_send(&packet, &reply_header, &(conn_info->peer_ip));
}

static void _tcp_receive_bogus_packet(tcp_Header* tcp_header,
    const tcp_ConnectionInfo *conn_info)
{
    tcp_Header reply_header;
    PacketBuf packet;

    tcp_make_reply_header(&reply_header, tcp_header, tcp_header->ack_no,
        tcp_header->seq_no, TCP_FLAG_RST | TCP_FLAG_ACK);
    
    pktbuf_create(&packet, STD_HEADROOM);
    tcp_send(&packet, &reply_header, &(conn_info->peer_ip));
}

static bool _tcp_conn_match(const tcp_ConnectionInfo *conn1,
    const tcp_ConnectionInfo *conn2)
{
    return
        ip_addr_equal(&conn1->peer_ip, &conn2->peer_ip) &&
        (conn1->peer_port == conn2->peer_port) &&
        (conn1->my_port == conn2->my_port);
}

/**
 * tcp_receive - Handles the receipt of a TCP segment
 * @packet: A packet buffer containing the segment
 * @ip_hdr: The extracted IP header
 */
void tcp_receive(PacketBuf *packet, ip_Header *ip_hdr)
{
    tcp_Header header;
    tcp_ConnectionInfo conn_info;
    uint8_t i;
    
    tcp_get_header(packet, &header);
    conn_info.peer_ip   = ip_hdr->source_ip;
    conn_info.peer_port = header.source_port;
    conn_info.peer_seq  = header.seq_no;
    conn_info.my_port   = header.dest_port;
    
    // Respond to connection requests
    if (tcp_is_connection_begin_seg(&header)) {
        if (tcp_port_is_open(header.dest_port)) {
            _tcp_accept_connection(&header, &conn_info);
        }
        return;
    }
    
    // Receive for current connection
    if ((tcp.current.flags & SF_EXISTS) &&
        _tcp_conn_match(&conn_info, &tcp.current.conn)) {
        _tcp_receive_for_current(&header, packet);
        return;
    }
    
    // Receive for pending connections
    for (i = 0; i < tcp.num_pending; i++) {
        if (_tcp_conn_match(&conn_info, &(tcp.pending[i]))) {
            _tcp_receive_for_pending(&header, &conn_info);
            return;
        }
    }

    // Reject bogus packet
    _tcp_receive_bogus_packet(&header, &conn_info);
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
    tcp.num_pending = 0;
    _tcp_reset_xmit_buffer();
}
