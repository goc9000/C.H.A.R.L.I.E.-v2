/*
 * eth.c - Ethernet protocol helper functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <string.h>
#include <ctype.h>

#include "eth.h"
#include "util/hex.h"

mac_addr_t MAC_ADDR_NULL = {0x00,0x00,0x00,0x00,0x00,0x00};
mac_addr_t MAC_ADDR_BCAST = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/**
 * eth_addr_equal - Test for MAC address equality
 * @addr1: The first MAC address
 * @addr2: The second MAC address
 */
bool eth_addr_equal(const mac_addr_t *addr1, const mac_addr_t *addr2)
{
    return !memcmp(addr1, addr2, sizeof(mac_addr_t));
}

/**
 * eth_addr_match - Test for MAC address equivalence
 * @addr1: The first MAC address
 * @addr2: The second MAC address
 * 
 * This also checks for at least one of the addresses being a
 * broadcast address.
 */
bool eth_addr_match(const mac_addr_t *addr1, const mac_addr_t *addr2)
{
    return eth_addr_equal(addr1, addr2)
        || eth_addr_equal(addr1, &MAC_ADDR_BCAST)
        || eth_addr_equal(addr2, &MAC_ADDR_BCAST);
}

/**
 * eth_format_mac - Formats a MAC address as text
 * @buf: A buffer to hold the string (at least 18 bytes long)
 * @addr: The MAC address to format
 */
void eth_format_mac(char *buf, const mac_addr_t *addr)
{
    const uint8_t *bytes = (uint8_t *)addr;
    uint8_t i;
    uint32_t value;
    
    for (i = 0; i < 6; i++) {
        value = bytes[i];
        hex_format(buf, 2, value);
        
        buf += 2;
        *(buf++) = ((i == 5) ? 0 : ':');
    }
}

/**
 * eth_parse_mac - Parses a MAC address
 * @addr: A variable to hold the address
 * @text: A text string of the form AA:BB:CC:DD:EE:FF
 */
bool eth_parse_mac(mac_addr_t *addr, const char *text)
{
    uint8_t *bytes = (uint8_t *)addr;
    uint8_t i;
    uint32_t value;
    
    for (i = 0; i < 6; i++) {
        if (hex_parse(text, 2, &value) != 2) {
            return FALSE;
        }
        bytes[i] = (uint8_t)value;
        
        text += 2;
        if (*(text++) != ((i == 5) ? 0 : ':')) {
            return FALSE;
        }
    }

    return TRUE;
}
