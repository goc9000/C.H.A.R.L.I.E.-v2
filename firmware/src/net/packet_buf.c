/*
 * packet_buf.c - Packet buffer data structure
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

/*
 * This structure is backed up by ENC28J60 memory; it is meant to be
 * used in a fashion similar to the "struct skt_buf" in the Linux kernel.
 */

#include <string.h>

#include "debug/debug.h"
#include "net/endian.h"
#include "net/dev/enc28j60.h"
#include "net/proto/ip.h"
#include "packet_buf.h"

#define AREA_COUNT 4

uint16_t __AREA_LIMITS[(AREA_COUNT+1) * 2] PROGMEM = {
    CFG_E28J_RECV_BUFFER_START, CFG_E28J_RECV_BUFFER_END,
    CFG_E28J_TRAN_BUFFER_START, CFG_E28J_TRAN_BUFFER_END,
     CFG_E28J_OOB_BUFFER_START,  CFG_E28J_OOB_BUFFER_END,
     CFG_E28J_AUX_BUFFER_START,  CFG_E28J_AUX_BUFFER_END,
                             0,                   0x1FFF
};

#define AREA_BOT(area) pgm_read_word(__AREA_LIMITS + ((area) << 1))
#define AREA_TOP(area) pgm_read_word(__AREA_LIMITS + ((area) << 1) + 1)

bool _write_sanity_check(const PacketBuf *packet, int16_t amount)
{
    if (pktbuf_area(packet) == PKTBUF_AREA_RECEIVED) {
        debug_printf("Attempted write to received packet!\n");
        return FALSE;
    }
    
    if (amount < 0) {
        if ((uint16_t)(-amount) > pktbuf_headroom(packet)) {
            debug_printf("Packet buffer headroom exceeded in area %d (%d > %d)\n",
                (int)pktbuf_area(packet), (int)-amount, pktbuf_headroom(packet));
            return FALSE;
        }
    } else {
        if ((uint16_t)amount > pktbuf_remaining(packet)) {
            debug_printf("Packet buffer overflow in area %d (%d > %d)\n",
                (int)pktbuf_area(packet), (int)amount, pktbuf_remaining(packet));
            return FALSE;
        }
    }
    
    return TRUE;
}

/**
 * pktbuf_create - Creates an empty packet buffer in the main area
 * @packet: The structure to be initialized
 * @head_room: How much space to allocate for headers
 */
void pktbuf_create(PacketBuf *packet, uint16_t head_room)
{
    pktbuf_create_in(packet, PKTBUF_AREA_NORMAL, head_room);
}

/**
 * pktbuf_create_type - Creates an empty packet buffer
 * @packet: The structure to be initialized
 * @area: The area in which to allocate the packet (a PKTBUF_AREA_* constant)
 * @head_room: How much space to allocate for headers
 */
void pktbuf_create_in(PacketBuf *packet, uint8_t area, uint16_t head_room)
{
    packet->start = AREA_BOT(area) + 1;
    packet->length = 0;
    
    pktbuf_reserve(packet, head_room);
}

/**
 * pktbuf_get_header - Extract header from packet buffer
 * @packet: The packet buffer
 * @header: A buffer to hold the header
 * @header_len: The header length in bytes
 * @ec_signature: The endianness conversion signature
 */
void pktbuf_get_header(PacketBuf *packet, void *header,
    uint8_t header_len, uint32_t ec_signature)
{
    enc28j60_read_mem_at(header, packet->start, header_len);
    endian_convert(header, ec_signature);
    
    pktbuf_pull(packet, header_len);
}

/**
 * pktbuf_get_token - Gets a token from the packet buffer
 * @packet: The packet buffer
 * @token: A buffer to store the token in. May be NULL if we just want
 *         to ignore the token
 * @max_tok: The buffer capacity, NUL included
 * @delims: A string containing all the delimiter chars
 * 
 * Returns the first delimiter char encountered, or NUL if the end
 * of the buffer occured. The delimiter char is not consumed.
 */
char pktbuf_get_token(PacketBuf *packet, char *token, uint16_t max_tok,
    PGM_P delims)
{
    char buf[16];
    char term = 0;
    uint16_t tok_size = 0;
    uint8_t i, count;
    
    max_tok--;
    
    while (packet->length && !term) {
        count = (packet->length > 16) ? 16 : packet->length;
        enc28j60_read_mem_at(buf, packet->start, count);
        
        for (i = 0; i < count; i++) {
            if (strchr_P(delims, buf[i])) {
                term = buf[i];
                break;
            }
            
            packet->length--;
            packet->start++;
            
            if (token && max_tok) {
                token[tok_size++] = buf[i];
                max_tok--;
            }
        }
    }
    
    token[tok_size] = 0;
    
    return term;
}

/**
 * pktbuf_add_header - Prepend header to packet buffer
 * @packet: The packet buffer
 * @header: A buffer holding the header
 * @header_len: The header length in bytes
 * @ec_signature: The endianness conversion signature
 */
void pktbuf_add_header(PacketBuf *packet, void *header,
    uint8_t header_len, uint32_t ec_signature)
{
    if (!_write_sanity_check(packet, (int16_t)0 - header_len)) {
        return;
    }
    
    packet->start -= header_len;
    packet->length += header_len;
    endian_convert(header, ec_signature);
    enc28j60_write_mem_at(header, packet->start, header_len);
    endian_convert(header, ec_signature);
}

/**
 * pktbuf_add_data - Append data to packet buffer
 * @packet: The packet buffer
 * @data: A buffer holding the data
 * @data_len: The data length in bytes
 */
void pktbuf_add_data(PacketBuf *packet, const void *data,
    uint16_t data_len)
{
    if (!_write_sanity_check(packet, data_len)) {
        return;
    }
    
    enc28j60_write_mem_at(data, packet->start + packet->length,
        data_len);
    packet->length += data_len;
}

/**
 * pktbuf_add_buf - Append data from a packet buffer to another
 * @packet: The packet buffer to add to
 * @from: The packed buffer to add from
 */
void pktbuf_add_buf(PacketBuf *packet, const PacketBuf *from)
{
    uint8_t buf[16];
    uint16_t length = from->length;
    uint16_t addr = from->start;
    uint16_t count;
    
    if (!_write_sanity_check(packet, length)) {
        return;
    }
    
    while (length) {
        count = (length > 16) ? 16 : length;
        enc28j60_read_mem_at(buf, addr, count);
        
        pktbuf_add_data(packet, buf, count);
        
        addr += 16;
        length -= count;
    }
}

/**
 * pktbuf_add_str - Append a string to a packet buffer
 * @packet: The packet buffer to add to
 * @str: The string to add
 */
void pktbuf_add_str(PacketBuf *packet, const char *str)
{
    if (!_write_sanity_check(packet, strlen(str))) {
        return;
    }
    
    pktbuf_add_data(packet, str, strlen(str));
}

/**
 * pktbuf_add_str_P - Append a program memory string to a packet buffer
 * @packet: The packet buffer to add to
 * @str: The string to add
 */
void pktbuf_add_str_P(PacketBuf *packet, PGM_P str)
{
    char buf[16];
    uint8_t in_buf = 0;
    char c;
    
    if (!_write_sanity_check(packet, strlen_P(str))) {
        return;
    }
    
    while (TRUE) {
        c = pgm_read_byte(str);
        str++;
        
        if (!c || (in_buf == 16)) {
            pktbuf_add_data(packet, buf, in_buf);
            in_buf = 0;
        }
        if (!c) {
            return;
        }
        
        buf[in_buf++] = c;
    }
}

/**
 * pktbuf_pull - Removes a number of bytes from the start of the buffer
 * @packet: The packet buffer
 * @amount: The number of bytes to remove
 */
void pktbuf_pull(PacketBuf *packet, uint16_t amount)
{
    packet->start = enc28j60_effective_addr(packet->start, amount);
    packet->length -= amount;
}

/**
 * pktbuf_reserve - Reserves a number of bytes for the headers
 * @packet: The packet buffer
 * @amount: The number of bytes to reserve
 * 
 * The buffer should be empty before doing this.
 */
void pktbuf_reserve(PacketBuf *packet, uint16_t amount)
{
    if (!_write_sanity_check(packet, amount)) {
        return;
    }
    
    packet->start += amount;
}

/**
 * pktbuf_retract - Removes a number of bytes from the end of the buffer
 * @packet: The packet buffer
 * @amount: The number of bytes to remove
 */
void pktbuf_retract(PacketBuf *packet, uint16_t amount)
{
    if (amount > packet->length) {
        amount = packet->length;
    }
    
    packet->length -= amount;
}

/**
 * pktbuf_trim - Trims the buffer to a specified length
 * @packet: The packet buffer
 * @length: The new length of the buffer
 */
void pktbuf_trim(PacketBuf *packet, uint16_t length)
{
    if (length < packet->length) {
        packet->length = length;
    }
}

/**
 * pktbuf_area - Gets the area where the buffer is allocated
 * @packet: The packet buffer
 */
uint8_t pktbuf_area(const PacketBuf *packet)
{
    uint8_t i;
    
    for (i = 0; i < AREA_COUNT; i++) {
        if ((packet->start >= AREA_BOT(i)) &&
            (packet->start <= AREA_TOP(i))) {
            return i;
        }
    }
    
    return PKTBUF_AREA_INVALID;
}

/**
 * pktbuf_empty - Checks whether a packet buffer is empty
 * @packet: The packet buffer
 */
bool pktbuf_empty(const PacketBuf *packet)
{
    return !packet->length;
}

/**
 * pktbuf_remaining - Gets the remaining capacity of the buffer
 * @packet: The packet buffer
 */
uint16_t pktbuf_remaining(const PacketBuf *packet)
{
    uint8_t area = pktbuf_area(packet);
    
    if ((area == PKTBUF_AREA_INVALID) || (area == PKTBUF_AREA_RECEIVED)) {
        return 0;
    }
    
    return AREA_TOP(area) + 1 - packet->start - packet->length;
}

/**
 * pktbuf_remaining_head - Gets the remaining headroom of the buffer
 * @packet: The packet buffer
 */
uint16_t pktbuf_headroom(const PacketBuf *packet)
{
    uint8_t area = pktbuf_area(packet);
    
    if ((area == PKTBUF_AREA_INVALID) || (area == PKTBUF_AREA_RECEIVED)) {
        return 0;
    }
    
    return (packet->start - (AREA_BOT(area) - 1));
}

/**
 * pktbuf_ip_checksum - Computes the IP checksum for the data in the
 *                      packet buffer
 * @packet: The packet buffer
 */
uint16_t pktbuf_ip_checksum(const PacketBuf *packet)
{
    uint8_t buf[16];
    uint16_t length = packet->length;
    uint16_t addr = packet->start;
    uint16_t csum = ~0;
    uint16_t count;
    
    while (length) {
        count = (length > 16) ? 16 : length;
        enc28j60_read_mem_at(buf, addr, count);
        
        csum = ip_join_checksums(csum, ip_checksum(buf, count));
        
        addr += 16;
        length -= count;
    }
    
    return csum;
}
