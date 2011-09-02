/*
 * packet_buf.h - Packet buffer data structure
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _PACKET_BUF_H
#define _PACKET_BUF_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#include "util/bool.h"

// ethernet + ip + tcp header
#define STD_HEADROOM (14+20+20)

#define PKTBUF_AREA_RECEIVED  0
#define PKTBUF_AREA_NORMAL    1
#define PKTBUF_AREA_OOB       2
#define PKTBUF_AREA_AUX       3
#define PKTBUF_AREA_INVALID   4

typedef struct {
    uint16_t start;
    uint16_t length;
} PacketBuf;

void pktbuf_create(PacketBuf *packet, uint16_t head_room);
void pktbuf_create_in(PacketBuf *packet, uint8_t area, uint16_t head_room);

void pktbuf_get_header(PacketBuf *packet, void *header,
    uint8_t header_len, uint32_t ec_signature);
char pktbuf_get_token(PacketBuf *packet, char *token, uint16_t max_tok,
    PGM_P delims);

void pktbuf_add_header(PacketBuf *packet, void *header,
    uint8_t header_len, uint32_t ec_signature);
void pktbuf_add_data(PacketBuf *packet, const void *data,
    uint16_t data_len);
void pktbuf_add_buf(PacketBuf *packet, const PacketBuf *from);
void pktbuf_add_str(PacketBuf *packet, const char *str);
void pktbuf_add_str_P(PacketBuf *packet, PGM_P str);

void pktbuf_pull(PacketBuf *packet, uint16_t amount);
void pktbuf_reserve(PacketBuf *packet, uint16_t amount);
void pktbuf_retract(PacketBuf *packet, uint16_t amount);
void pktbuf_trim(PacketBuf *packet, uint16_t length);

uint8_t pktbuf_area(const PacketBuf *packet);
bool pktbuf_empty(const PacketBuf *packet);
uint16_t pktbuf_headroom(const PacketBuf *packet);
uint16_t pktbuf_remaining(const PacketBuf *packet);
uint16_t pktbuf_ip_checksum(const PacketBuf *packet);

#endif
