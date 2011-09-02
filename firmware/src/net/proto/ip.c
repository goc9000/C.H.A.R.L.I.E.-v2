/*
 * ip.c - IP protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "ip.h"

ip_addr_t IP_ADDR_NULL  = {0x00,0x00,0x00,0x00};
ip_addr_t IP_ADDR_BCAST = {0xFF,0xFF,0xFF,0xFF};

/**
 * ip_is_broadcast_ip - Checks for a local or global broadcast IP
 * @addr: An IP address
 * @net_mask: The netmask
 */
bool ip_is_broadcast_ip(const ip_addr_t *addr, uint8_t net_mask)
{
    ip_addr_t local_bcast_ip;
    
    ip_make_bcast_addr(&local_bcast_ip, addr, net_mask);
    
    return
        ip_addr_equal(addr, &IP_ADDR_BCAST) ||
        ip_addr_equal(addr, &local_bcast_ip);
}

/**
 * ip_addr_equal - Check for equality between two IP address
 * @addr1: An IP address
 * @addr2: Another IP address
 */
bool ip_addr_equal(const ip_addr_t *addr1, const ip_addr_t *addr2)
{
    return !memcmp(addr1, addr2, sizeof(ip_addr_t));
}

/**
 * ip_addr_match - Check for match between two IP address
 * @addr1: An IP address
 * @addr2: Another IP address
 * @net_mask: The netmask
 */
bool ip_addr_match(const ip_addr_t *addr1, const ip_addr_t *addr2,
    uint8_t net_mask)
{
    return ip_addr_equal(addr1, addr2) ||
        ip_is_broadcast_ip(addr1, net_mask) ||
        ip_is_broadcast_ip(addr2, net_mask);
}

/**
 * ip_same_network - Tests whether two IPs belong to the same network
 * @addr1: An IP address
 * @addr2: Another IP address
 * @net_mask: The netmask
 */
bool ip_same_network(const ip_addr_t *addr1, const ip_addr_t *addr2,
    uint8_t net_mask)
{
    const uint8_t *bytes_a = (const uint8_t *)addr1;
    const uint8_t *bytes_b = (const uint8_t *)addr2;
    uint8_t i;
    
    for (i = 0; i < 4; i++) {
        if ((bytes_a[i] ^ bytes_b[i]) & ip_get_mask_byte(net_mask, i)) {
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * ip_make_bcast_addr - Gets the broadcast address for an IP
 * @bcast_addr: A variable to receive the address
 * @base_addr: The base address
 * @net_mask: The netmask
 */
void ip_make_bcast_addr(ip_addr_t *bcast_addr, const ip_addr_t *base_addr,
    uint8_t net_mask)
{
    uint8_t *bytes_b = (uint8_t *)bcast_addr;
    const uint8_t *bytes_n = (const uint8_t *)base_addr;
    uint8_t i;
    
    for (i = 0; i < 4; i++) {
        bytes_b[i] = bytes_n[i] | ~ip_get_mask_byte(net_mask, i);
    }
}

/**
 * ip_get_mask_byte - Gets byte in given netmask
 * @mask: The netmask
 * @index: The byte index
 */
uint8_t ip_get_mask_byte(uint8_t mask, uint8_t index)
{
    uint8_t left, pos = index << 3;

    if (pos > mask) {
        return 0x00;
    }

    left = mask - pos;
    if (left > 7) {
        return 0xFF;
    }

    return ~(_BV(8 - left) - 1);
}

/**
 * ip_decode_mask - Decodes a netmask given in dotted-quad format
 * @net_mask: The full netmask
 * 
 * A number between 0 and 32 is returned.
 */
uint8_t ip_decode_net_mask(const ip_addr_t *net_mask)
{
    const uint8_t *bytes = (const uint8_t *)net_mask;
    uint8_t dribble, i, mask = 0;

    for (i=0; i < 4; i++) {
        if (bytes[i] != 0xFF) {
            break;
        }
        mask += 8;
    }

    if (i == 4) {
        return mask;
    }

    dribble = bytes[i];
    for (i = 7; i > 0; i--) {
        if (!(dribble & _BV(i))) {
            break;
        }
        mask++;
    }

    return mask;
}

/**
 * ip_format - Formats an IP address as text
 * @buf: A buffer to receive the text; must be at least 16 chars wide
 * @addr: The IP address to be formatted
 */
void ip_format(char* buf, const ip_addr_t *addr)
{
    const uint8_t *bytes = (const uint8_t *)addr;
    uint8_t i;
    
    for (i = 0; i < 4; i++)
    {
        itoa(bytes[i], buf, 10);
        buf += strlen(buf);
        if (i < 3) {
            *buf = '.';
        }
        buf++;
    }
}

/**
 * ip_parse - Parses an IP address given in dotted-quad text format
 * @addr: A variable to hold the address
 * @text: The address text
 */
bool ip_parse(ip_addr_t *addr, const char *text)
{
    uint8_t *bytes = (uint8_t *)addr;
    uint8_t p = 0, digits = 0;
    uint16_t temp = 0;

    while (TRUE) {
        char c = *(text++);

        if ((c >= '0') && (c <= '9')) {
            temp = temp * 10 + (c - '0');
            if (temp > 255) {
                return FALSE;
            }
            digits++;
        } else if (c == ((p == 3) ? 0 : '.')) {
            if (!digits) {
                return FALSE;
            }
            bytes[p++] = temp;
            if (p == 4) {
                break;
            }
            temp = digits = 0;
        } else {
            return FALSE;
        }
    }
    
    return TRUE;
}

/**
 * sum1c - Adds numbers in one's complement arithmetic
 * @a: A number
 * @b: Another number
 */
uint16_t sum1c(uint16_t a, uint16_t b)
{
    uint32_t sum = (uint32_t)a + b;

    return (sum & 0xFFFF) + (sum >> 16);
}

/**
 * ip_checksum - Computes the IP checksum for a block of data
 * @data: A buffer holding the data block
 * @data_size: The length of the data block
 * 
 * Keep in mind that a header needs to be converted to network
 * endianness before applying this function to it.
 */
uint16_t ip_checksum(const void *data, uint16_t data_size)
{
    const uint16_t *ptr = (const uint16_t *)data;
    uint16_t sum = 0;
    for (; data_size >= 2; data_size -= 2) {
        sum = sum1c(sum, *ptr++);
    }

    if (data_size) {
        sum = sum1c(sum, (*ptr) & 0x00FF);
    }

    return ~sum;
}

/**
 * ip_join_checksums - Joins the IP checksums for adjacent data blocks
 * @sum_a: The checksum for the first block
 * @sum_b: The checksum for the second block
 */
uint16_t ip_join_checksums(uint16_t sum_a, uint16_t sum_b)
{
    return ~sum1c(~sum_a, ~sum_b);
}

/**
 * ip_update_checksum - Update an IP checksum following a change in
 *                      one of the words in the data block
 * @checksum: The initial checksum
 * @old_field_val: The old field value
 * @new_field_val: The new field value
 * 
 * The formula used here is explained in RFC 1624.
 */
uint16_t ip_update_checksum(uint16_t checksum, uint16_t old_field_val,
    uint16_t new_field_val)
{
    return ~sum1c(sum1c(~checksum, ~old_field_val), new_field_val);
}

/**
 * ip_get_header - Extract IP header from packet
 * @packet: A packet buffer containing an IP packet
 * @ip_hdr: A structure to store the extracted header
 */
void ip_get_header(PacketBuf *packet, ip_Header *ip_hdr)
{
    uint8_t hdr_size;
    
    pktbuf_get_header(packet, ip_hdr, sizeof(ip_Header), EC_SIG_IP);
    
    hdr_size = ((uint8_t)ip_hdr->ihl) << 2;
    pktbuf_pull(packet, hdr_size - sizeof(ip_Header));
    pktbuf_trim(packet, ip_hdr->length - hdr_size);
}

/**
 * ip_update_hdr_checksum - Updates the checksum in an IP header
 * @header: An IP header
 * 
 * Note: the header must be stored using host endianness.
 */
void ip_update_hdr_checksum(ip_Header *header)
{
    endian_convert(header, EC_SIG_IP);
    header->checksum = 0;
    header->checksum = ip_checksum(header, sizeof(ip_Header));
    endian_convert(header, EC_SIG_IP);
}

/**
 * ip_is_fragment - Checks whether a header represents an IP fragment
 */
bool ip_is_fragment(const ip_Header *header)
{
    return (header->fragments & (IP_FRAG_OFFS_MASK | IP_FRAG_MORE_FRAGMENTS));
}

