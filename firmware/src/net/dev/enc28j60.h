/*
 * enc28j60.h - ENC28J60 Ethernet controller functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _ENC28J60_H
#define _ENC28J60_H

#include <avr/io.h>

#include "net/packet_buf.h"
#include "net/proto/eth.h"
#include "util/error.h"
#include "util/bool.h"
#include "dot_config.h"

typedef uint16_t enc28j60_memptr_t;

// Frame receive status bits
#define ENC28J60_RECV_STAT_VLAN_BIT    14
#define ENC28J60_RECV_STAT_UNRECOG_BIT 13
#define ENC28J60_RECV_STAT_PAUSE_BIT   12
#define ENC28J60_RECV_STAT_CTRL_BIT    11
#define ENC28J60_RECV_STAT_DRIBBLE_BIT 10
#define ENC28J60_RECV_STAT_BCAST_BIT    9
#define ENC28J60_RECV_STAT_MCAST_BIT    8
#define ENC28J60_RECV_STAT_OK_BIT       7
#define ENC28J60_RECV_STAT_ETHTYPE_BIT  6
#define ENC28J60_RECV_STAT_LENERR_BIT   5
#define ENC28J60_RECV_STAT_CRCERR_BIT   4
#define ENC28J60_RECV_STAT_CARREVT_BIT  2
#define ENC28J60_RECV_STAT_DROPEVT_BIT  0

// Internal constants
#define E28J_BANK_BIT                   5
#define E28J_IN_BANK_MASK               (_BV(E28J_BANK_BIT)-1)

// Time constants
#define E28J_TCSS_US                    2
#define E28J_TCSH_US                    2
#define E28J_RESET_PULSE_TIME_US        5
#define E28J_HARD_RESET_TIME_US         1000
#define E28J_SOFT_RESET_TIME_US         200
#define E28J_PHY_REG_DELAY_US           15

// Operation codes
#define E28J_OP_READ_REG                0x00
#define E28J_OP_WRITE_REG               0x40
#define E28J_OP_SET_BITS                0x80
#define E28J_OP_CLEAR_BITS              0xA0
#define E28J_OP_READ_MEM                0x3A
#define E28J_OP_WRITE_MEM               0x7A
#define E28J_OP_SOFT_RESET              0xFF

// Bank 0 registers
#define E28J_ERDPT                      0x00
#define E28J_ERDPTL                     0x00
#define E28J_ERDPTH                     0x01
#define E28J_EWRPT                      0x02
#define E28J_EWRPTL                     0x02
#define E28J_EWRPTH                     0x03
#define E28J_ETXST                      0x04
#define E28J_ETXSTL                     0x04
#define E28J_ETXSTH                     0x05
#define E28J_ETXND                      0x06
#define E28J_ETXNDL                     0x06
#define E28J_ETXNDH                     0x07
#define E28J_ERXST                      0x08
#define E28J_ERXSTL                     0x08
#define E28J_ERXSTH                     0x09
#define E28J_ERXND                      0x0A
#define E28J_ERXNDL                     0x0A
#define E28J_ERXNDH                     0x0B
#define E28J_ERXRDPT                    0x0C
#define E28J_ERXRDPTL                   0x0C
#define E28J_ERXRDPTH                   0x0D
#define E28J_ERXWRPT                    0x0E
#define E28J_ERXWRPTL                   0x0E
#define E28J_ERXWRPTH                   0x0F
#define E28J_EDMAST                     0x10
#define E28J_EDMASTL                    0x10
#define E28J_EDMASTH                    0x11
#define E28J_EDMAND                     0x12
#define E28J_EDMANDL                    0x12
#define E28J_EDMANDH                    0x13
#define E28J_EDMADST                    0x14
#define E28J_EDMADSTL                   0x14
#define E28J_EDMADSTH                   0x15
#define E28J_EDMACS                     0x16
#define E28J_EDMACSL                    0x16
#define E28J_EDMACSH                    0x17
#define E28J_EIE                        0x1B
#define E28J_EIR                        0x1C
#define E28J_ESTAT                      0x1D
#define E28J_ECON2                      0x1E
#define E28J_ECON1                      0x1F

// Bank 1 registers
#define E28J_EHT0                       0x20
#define E28J_EHT1                       0x21
#define E28J_EHT2                       0x22
#define E28J_EHT3                       0x23
#define E28J_EHT4                       0x24
#define E28J_EHT5                       0x25
#define E28J_EHT6                       0x26
#define E28J_EHT7                       0x27
#define E28J_EPMM0                      0x28
#define E28J_EPMM1                      0x29
#define E28J_EPMM2                      0x2A
#define E28J_EPMM3                      0x2B
#define E28J_EPMM4                      0x2C
#define E28J_EPMM5                      0x2D
#define E28J_EPMM6                      0x2E
#define E28J_EPMM7                      0x2F
#define E28J_EPMCS                      0x30
#define E28J_EPMCSL                     0x30
#define E28J_EPMCSH                     0x31
#define E28J_EPMOL                      0x34
#define E28J_EPMOH                      0x35
#define E28J_EWOLIE                     0x36
#define E28J_EWOLIR                     0x37
#define E28J_ERXFCON                    0x38
#define E28J_EPKTCNT                    0x39

// Bank 2 registers
#define E28J_MACON1                     0x40
#define E28J_MACON2                     0x41
#define E28J_MACON3                     0x42
#define E28J_MACON4                     0x43
#define E28J_MABBIPG                    0x44
#define E28J_MAIPG                      0x46
#define E28J_MAIPGL                     0x46
#define E28J_MAIPGH                     0x47
#define E28J_MACLCON1                   0x48
#define E28J_MACLCON2                   0x49
#define E28J_MAMXFL                     0x4A
#define E28J_MAMXFLL                    0x4A
#define E28J_MAMXFLH                    0x4B
#define E28J_MAPHSUP                    0x4D
#define E28J_MICON                      0x51
#define E28J_MICMD                      0x52
#define E28J_MIREGADR                   0x54
#define E28J_MIWR                       0x56
#define E28J_MIWRL                      0x56
#define E28J_MIWRH                      0x57
#define E28J_MIRD                       0x58
#define E28J_MIRDL                      0x58
#define E28J_MIRDH                      0x59

// Bank 3 registers
#define E28J_MAADR1                     0x60
#define E28J_MAADR0                     0x61
#define E28J_MAADR3                     0x62
#define E28J_MAADR2                     0x63
#define E28J_MAADR5                     0x64
#define E28J_MAADR4                     0x65
#define E28J_EBSTSD                     0x66
#define E28J_EBSTCON                    0x67
#define E28J_EBSTCS                     0x68
#define E28J_EBSTCSL                    0x68
#define E28J_EBSTCSH                    0x69
#define E28J_MISTAT                     0x6A
#define E28J_EREVID                     0x72
#define E28J_ECOCON                     0x75
#define E28J_EFLOCON                    0x77
#define E28J_EPAUS                      0x78
#define E28J_EPAUSL                     0x78
#define E28J_EPAUSH                     0x79

// PHY registers
#define E28J_PHCON1                     0x00
#define E28J_PHSTAT1                    0x01
#define E28J_PHID1                      0x02
#define E28J_PHID2                      0x03
#define E28J_PHCON2                     0x10
#define E28J_PHSTAT2                    0x11
#define E28J_PHIE                       0x12
#define E28J_PHIR                       0x13
#define E28J_PHLCON                     0x14

// Bit values for register 0x1B: EIE
#define E28J_INTIE                      7
#define E28J_PKTIE                      6
#define E28J_DMAIE                      5
#define E28J_LINKIE                     4
#define E28J_TXIE                       3
#define E28J_WOLIE                      2
#define E28J_TXERIE                     1
#define E28J_RXERIE                     0

// Bit values for register 0x1C: EIR
#define E28J_PKTIF                      6
#define E28J_DMAIF                      5
#define E28J_LINKIF                     4
#define E28J_TXIF                       3
#define E28J_WOLIF                      2
#define E28J_TXERIF                     1
#define E28J_RXERIF                     0

// Bit values for register 0x1D: ESTAT
#define E28J_INT                        7
#define E28J_LATECOL                    4
#define E28J_RXBUSY                     2
#define E28J_TXABRT                     1
#define E28J_CLKRDY                     0

// Bit values for register 0x1E: ECON2
#define E28J_AUTOINC                    7
#define E28J_PKTDEC                     6
#define E28J_PWRSV                      5
#define E28J_VRPS                       3

// Bit values for register 0x1F: ECON1
#define E28J_TXRST                      7
#define E28J_RXRST                      6
#define E28J_DMAST                      5
#define E28J_CSUMEN                     4
#define E28J_TXRTS                      3
#define E28J_RXEN                       2
#define E28J_BSEL1                      1
#define E28J_BSEL0                      0

// Bit values for register 0x38: ERXFCON
#define E28J_UCEN                       7
#define E28J_ANDOR                      6
#define E28J_CRCEN                      5
#define E28J_PMEN                       4
#define E28J_MPEN                       3
#define E28J_HTEN                       2
#define E28J_MCEN                       1
#define E28J_BCEN                       0

// Bit values for register 0x40: MACON1
#define E28J_LOOPBK                     4
#define E28J_TXPAUS                     3
#define E28J_RXPAUS                     2
#define E28J_PASSALL                    1
#define E28J_MARXEN                     0

// Bit values for register 0x41: MACON2
#define E28J_MARST                      7
#define E28J_RNDRST                     6
#define E28J_MARXRST                    3
#define E28J_RFUNRST                    2
#define E28J_MATXRST                    1
#define E28J_TFUNRST                    0

// Bit values for register 0x42: MACON3
#define E28J_PADCFG2                    7
#define E28J_PADCFG1                    6
#define E28J_PADCFG0                    5
#define E28J_TXCRCEN                    4
#define E28J_PHDRLEN                    3
#define E28J_HFRMEN                     2
#define E28J_FRMLNEN                    1
#define E28J_FULDPX                     0

// Bit values for register 0x43: MACON4
#define E28J_DEFER                      6
#define E28J_BPEN                       5
#define E28J_NOBKOFF                    4
#define E28J_LONGPRE                    1
#define E28J_PUREPRE                    0

// Bit values for register 0x52: MICMD
#define E28J_MIISCAN                    1
#define E28J_MIIRD                      0

// Bit values for register 0x6A: MISTAT
#define E28J_NVALID                     2
#define E28J_SCAN                       1
#define E28J_BUSY                       0

// Bit values for PHY register 0x00: PHCON1
#define E28J_PRST                      15
#define E28J_PLOOPBK                   14
#define E28J_PPWRSV                    11
#define E28J_PDPXMD                     8

// Bit values for PHY register 0x01: PHSTAT1
#define E28J_PFDPX                     12
#define E28J_PHDPX                     11
#define E28J_LLSTAT                     2
#define E28J_JBSTAT                     1

// Bit values for PHY register 0x10: PHCON2
#define E28J_FRCLNK                    15
#define E28J_TXDIS                     14
#define E28J_JABBER                    10
#define E28J_HDLDIS                     8

// Bit values for PHY register 0x11: PHSTAT2
#define E28J_TXSTAT                    13
#define E28J_RXSTAT                    12
#define E28J_COLSTAT                   11
#define E28J_LSTAT                     10
#define E28J_DPXSTAT                    9
#define E28J_PLRITY                     4

// Bit values for PHY register 0x12: PHIE
#define E28J_PLNKIE                     4
#define E28J_PGEIE                      1

// Bit values for PHY register 0x13: PHIR
#define E28J_PLNKIF                     4
#define E28J_PGIF                       2

// Bit values for PHY register 0x14: PHLCON
#define E28J_LACFG3                    11
#define E28J_LACFG2                    10
#define E28J_LACFG1                     9
#define E28J_LACFG0                     8
#define E28J_LBCFG3                     7
#define E28J_LBCFG2                     6
#define E28J_LBCFG1                     5
#define E28J_LBCFG0                     4
#define E28J_LFRQ1                      3
#define E28J_LFRQ0                      2
#define E28J_STRCH                      1

// Bit values for the packet transmission control byte
#define E28J_PHUGEEN                    3
#define E28J_PPADEN                     2
#define E28J_PCRCEN                     1
#define E28J_POVERRIDE                  0

// Complex operations
err_t enc28j60_init(mac_addr_t *mac_addr);
uint8_t enc28j60_get_revision_id(void);
void enc28j60_set_mac_addr(const mac_addr_t *mac_addr);
void enc28j60_get_mac_addr(mac_addr_t *mac_addr);
uint8_t enc28j60_get_pending_frames(void);
bool enc28j60_get_received_frame(PacketBuf *packet, uint32_t *recv_status);
void enc28j60_free_received_frame(void);
void enc28j60_alloc_packet(PacketBuf *packet, int head_room);
void enc28j60_transmit(const PacketBuf *packet);
bool enc28j60_check_link_up(void);

// 1st order primitives
uint8_t enc28j60_read_reg(uint8_t reg);
void enc28j60_write_reg(uint8_t reg, uint8_t value);
void enc28j60_set_reg_bits(uint8_t reg, uint8_t bits);
void enc28j60_clear_reg_bits(uint8_t reg, uint8_t bits);
void enc28j60_read_mem(void *buf, uint16_t length);
void enc28j60_write_mem(const void *buf, uint16_t length);
void enc28j60_soft_reset(void);

// 2nd order primitives
void enc28j60_hard_reset(void);
uint16_t enc28j60_read_reg_16bit(uint8_t reg_l);
void enc28j60_write_reg_16bit(uint8_t reg_l, uint16_t value);
uint16_t enc28j60_read_phy_reg(uint8_t reg);
void enc28j60_write_phy_reg(uint8_t reg, uint16_t value);
void enc28j60_read_mem_at(void *buf, enc28j60_memptr_t addr,
    uint16_t length);
void enc28j60_write_mem_at(const void *buf, enc28j60_memptr_t addr,
    uint16_t length);
void enc28j60_copy_mem(enc28j60_memptr_t dest_addr,
    enc28j60_memptr_t src_addr, uint16_t length);
enc28j60_memptr_t enc28j60_effective_addr(
    enc28j60_memptr_t base_addr, uint16_t offset);

#endif
