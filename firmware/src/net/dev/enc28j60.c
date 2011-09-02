/*
 * enc28j60.c - ENC28J60 Ethernet controller functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */
 
#include <util/delay.h>
#include <string.h>

#include "interf/spi.h"
#include "enc28j60.h"

static struct {
    uint8_t    reg_bank    : 2;
    mac_addr_t current_mac;
} enc28j60;

static void _enc28j60_chip_select(void)
{
    CFG_E28J_PORT &= ~(_BV(CFG_E28J_CS_BIT));
    _delay_us(E28J_TCSS_US);
}

static void _enc28j60_chip_deselect(void)
{
    _delay_us(E28J_TCSH_US);
    CFG_E28J_PORT |= _BV(CFG_E28J_CS_BIT);
    spi_write(0xFF);
}

/**
 * _enc28j60_is_super_reg - Checks for a 'super-register'
 * @reg: A register code to test
 * 
 * A 'super-register' is one accessible from any register bank.
 */
static bool _enc28j60_is_super_reg(uint8_t reg)
{
    return ((reg & E28J_IN_BANK_MASK) >= 0x1B);
}

static bool _enc28j60_is_mac_reg(uint8_t reg)
{
    return
        ((reg >= 0x40) && (reg <= 0x59)) ||
        ((reg >= 0x60) && (reg <= 0x65)) ||
        (reg == 0x6A);
}

static void _enc28j60_reg_op_preamble(uint8_t op, uint8_t reg)
{
    // Select correct register bank
    uint8_t bank = (reg >> E28J_BANK_BIT) & 0x03;

    if (!_enc28j60_is_super_reg(reg) && (bank != enc28j60.reg_bank))
    {
        _enc28j60_chip_select();
        spi_write(E28J_OP_CLEAR_BITS | (E28J_ECON1 & E28J_IN_BANK_MASK));
        spi_write(0x03);
        _enc28j60_chip_deselect();
        _enc28j60_chip_select();
        spi_write(E28J_OP_SET_BITS | (E28J_ECON1 & E28J_IN_BANK_MASK));
        spi_write(bank);
        _enc28j60_chip_deselect();
        enc28j60.reg_bank = bank;
    }

    _enc28j60_chip_select();
    spi_write(op | (reg & E28J_IN_BANK_MASK));
}

/**
 * enc28j60_read_reg - Read from an 8-bit ENC28J60 register
 * @reg: The register code
 */
uint8_t enc28j60_read_reg(uint8_t reg)
{
    uint8_t value;
    
    _enc28j60_reg_op_preamble(E28J_OP_READ_REG, reg);
    
    if (_enc28j60_is_mac_reg(reg)) {
        spi_write(0x00);
    }
    spi_read(value);
    
    _enc28j60_chip_deselect();

    return value;
}

/**
 * enc28j60_write_reg - Write to an 8-bit ENC28J60 register
 * @reg: The register code
 * @value: The value to write
 */
void enc28j60_write_reg(uint8_t reg, uint8_t value)
{
    _enc28j60_reg_op_preamble(E28J_OP_WRITE_REG, reg);
    spi_write(value);
    _enc28j60_chip_deselect();
}

/**
 * enc28j60_set_reg_bits - Set bits in an ENC28J60 register
 * @reg: The register code
 * @value: A mask containting the bits to set
 * 
 * Cannot be used for MAC registers.
 */
void enc28j60_set_reg_bits(uint8_t reg, uint8_t bits)
{
    _enc28j60_reg_op_preamble(E28J_OP_SET_BITS, reg);
    spi_write(bits);
    _enc28j60_chip_deselect();
}

/**
 * enc28j60_clear_reg_bits - Clear bits in an ENC28J60 register
 * @reg: The register code
 * @value: A mask containting the bits to clear
 * 
 * Cannot be used for MAC registers.
 */
void enc28j60_clear_reg_bits(uint8_t reg, uint8_t bits)
{
    _enc28j60_reg_op_preamble(E28J_OP_CLEAR_BITS, reg);
    spi_write(bits);
    _enc28j60_chip_deselect();
}

/**
 * enc28j60_read_mem - Read ENC28J60 memory from current position
 * @buf: A buffer to store the read data
 * @length: The number of bytes to read
 * 
 * The data is read from the current memory pointer in ERDPT
 */
void enc28j60_read_mem(void *buf, uint16_t length)
{
    _enc28j60_chip_select();
    spi_write(E28J_OP_READ_MEM);
    while (length--) {
        spi_read(*((uint8_t *)buf++));
    }
    _enc28j60_chip_deselect();
}

/**
 * enc28j60_write_mem - Write ENC28J60 memory from current position
 * @buf: A buffer containing the data to write
 * @length: The number of bytes to write
 * 
 * The data is written from the current memory pointer in EWRPT
 */
void enc28j60_write_mem(const void *buf, uint16_t length)
{
    _enc28j60_chip_select();
    spi_write(E28J_OP_WRITE_MEM);
    while (length--) {
        spi_write(*((uint8_t *)buf++));
    }
    _enc28j60_chip_deselect();
}

/**
 * enc28j60_soft_reset - Send a reset command to the ENC28J60
 */
void enc28j60_soft_reset(void)
{
    _enc28j60_chip_select();
    spi_write(E28J_OP_SOFT_RESET);
    _enc28j60_chip_deselect();
    _delay_us(E28J_SOFT_RESET_TIME_US);
    
    enc28j60.reg_bank = 0;
}

/**
 * enc28j60_hard_reset - Hard reset the ENC28J60 via the RESET pin
 */
void enc28j60_hard_reset(void)
{
    CFG_E28J_PORT &= ~_BV(CFG_E28J_RESET_BIT);
    _delay_us(E28J_RESET_PULSE_TIME_US);
    CFG_E28J_PORT |= _BV(CFG_E28J_RESET_BIT);
    _delay_us(E28J_HARD_RESET_TIME_US);
    
    enc28j60.reg_bank = 0;
}

/**
 * enc28j60_read_reg_16bit - Read from a 16-bit ENC28J60 register
 * @reg_l: The code for the LSB half of the register
 */
uint16_t enc28j60_read_reg_16bit(uint8_t reg_l)
{
    return
        (enc28j60_read_reg(reg_l + 1) << 8) +
         enc28j60_read_reg(reg_l);
}

/**
 * enc28j60_write_reg_16bit - Write to a 16-bit ENC28J60 register
 * @reg_l: The code for the LSB half of the register
 * @value: The value to write
 */
void enc28j60_write_reg_16bit(uint8_t reg_l, uint16_t value)
{
    enc28j60_write_reg(reg_l, value & 0xFF);
    enc28j60_write_reg(reg_l + 1, value >> 8);
}

/**
 * enc28j60_read_phy_reg - Read from a PHY register
 * @reg: The PHY register code
 */
uint16_t enc28j60_read_phy_reg(uint8_t reg)
{
    enc28j60_write_reg(E28J_MIREGADR, reg);
    enc28j60_write_reg(E28J_MICMD, _BV(E28J_MIIRD));

    _delay_us(E28J_PHY_REG_DELAY_US);
    while (enc28j60_read_reg(E28J_MISTAT) & _BV(E28J_BUSY));

    enc28j60_write_reg(E28J_MICMD, 0);

    return enc28j60_read_reg_16bit(E28J_MIRD);
}

/**
 * enc28j60_write_phy_reg - Read from a PHY register
 * @reg: The PHY register code
 * @value: The value to write
 */
void enc28j60_write_phy_reg(uint8_t reg, uint16_t value)
{
    enc28j60_write_reg(E28J_MIREGADR, reg);
    enc28j60_write_reg_16bit(E28J_MIWR, value);

    _delay_us(E28J_PHY_REG_DELAY_US);
    while (enc28j60_read_reg(E28J_MISTAT) & _BV(E28J_BUSY));
}

/**
 * enc28j60_read_mem_at - Read ENC28J60 memory
 * @buf: A buffer to store the read data
 * @addr: The ENC28J60 memory address to read from
 * @length: The number of bytes to read
 */
void enc28j60_read_mem_at(void *buf, enc28j60_memptr_t addr, uint16_t length)
{
    enc28j60_write_reg_16bit(E28J_ERDPT, addr);
    enc28j60_read_mem(buf, length);
}

/**
 * enc28j60_write_mem_at - Write ENC28J60 memory
 * @buf: A buffer containing the data to write
 * @addr: The ENC28J60 memory address to write from
 * @length: The number of bytes to write
 */
void enc28j60_write_mem_at(const void *buf, enc28j60_memptr_t addr, uint16_t length)
{
    enc28j60_write_reg_16bit(E28J_EWRPT, addr);
    enc28j60_write_mem(buf, length);
}

/**
 * enc28j60_copy_mem - Copies data within ENC28J60 memory
 * @dest_addr: The destination address in ENC28J60 memory
 * @src_addr: The sources address in ENC28J60 memory
 * @length: The number of bytes to copy
 * 
 * The data blocks must not overlap.
 */
void enc28j60_copy_mem(enc28j60_memptr_t dest_addr,
    enc28j60_memptr_t src_addr, uint16_t length)
{
    uint8_t buf[32];
    uint16_t count;
    
    while (length) {
        count = length;
        if (count > 32) {
            count = 32;
        }

        enc28j60_read_mem_at(buf, src_addr, count);
        enc28j60_write_mem_at(buf, dest_addr, count);

        src_addr += count;
        dest_addr += count;
        length -= count;
    }
}

/**
 * enc28j60_effective_addr - Computes effective address after an offset
 *                           is applied
 * @base_addr: The base address
 * @offset: The applied offset
 * 
 * The function takes into account that the address space within the
 * read buffer wraps.
 */
enc28j60_memptr_t enc28j60_effective_addr(enc28j60_memptr_t base_addr,
    uint16_t offset)
{
    if ( //(base_addr >= CFG_E28J_RECV_BUFFER_START) && // not needed, always 0
        (base_addr <= CFG_E28J_RECV_BUFFER_END)) {
        base_addr += offset;
        while (base_addr > CFG_E28J_RECV_BUFFER_END) {
            base_addr -= (CFG_E28J_RECV_BUFFER_END - CFG_E28J_RECV_BUFFER_START + 1);
        }
        
        return base_addr;
    } else {
        return base_addr + offset;
    }
}

/**
 * enc28j60_get_revision_id - Gets the chip revision ID
 */
uint8_t enc28j60_get_revision_id(void)
{
    return enc28j60_read_reg(E28J_EREVID);
}

/**
 * enc28j60_set_mac_addr - Sets the ENC28J60's MAC address
 * @mac_addr: The new MAC address
 */
void enc28j60_set_mac_addr(const mac_addr_t *mac_addr)
{
    enc28j60.current_mac = *mac_addr;

    enc28j60_write_reg(E28J_MAADR0, mac_addr->o5);
    enc28j60_write_reg(E28J_MAADR1, mac_addr->o4);
    enc28j60_write_reg(E28J_MAADR2, mac_addr->o3);
    enc28j60_write_reg(E28J_MAADR3, mac_addr->o2);
    enc28j60_write_reg(E28J_MAADR4, mac_addr->o1);
    enc28j60_write_reg(E28J_MAADR5, mac_addr->o0);
}

/**
 * enc28j60_get_mac_addr - Gets the ENC28J60's MAC address
 * @mac_addr: A variable to store the address
 */
void enc28j60_get_mac_addr(mac_addr_t *mac_addr)
{
    *mac_addr = enc28j60.current_mac;
}

/**
 * enc28j60_get_pending_frames - Get the no. of frames in the receive queue
 */
uint8_t enc28j60_get_pending_frames(void)
{
    return enc28j60_read_reg(E28J_EPKTCNT);
}

/**
 * enc28j60_get_received_frame - Gets the frame at the front of the
 *                               receive queue as a packet buffer struct
 * @packet: A structure to receive the frame
 * @recv_status: A variable to be loaded with the receive status
 */
bool enc28j60_get_received_frame(PacketBuf *packet, uint32_t *recv_status)
{
    enc28j60_memptr_t frame_addr;
    
    if (!enc28j60_get_pending_frames()) {
        return FALSE;
    }

    frame_addr = enc28j60_read_reg_16bit(E28J_ERXRDPT) + 2;
    enc28j60_read_mem_at(recv_status, frame_addr, 4);
    
    packet->start = frame_addr+4;
    packet->length = ((*recv_status & 0xFFFF) - 4);
    
    return TRUE;
}

/**
 * enc28j60_free_received_frame - Frees the frame at the front of the receive queue
 */
void enc28j60_free_received_frame(void)
{
    enc28j60_memptr_t frame_addr = enc28j60_read_reg_16bit(E28J_ERXRDPT);
    enc28j60_memptr_t next_frame_addr;

    enc28j60_read_mem_at((uint8_t *)&next_frame_addr, frame_addr, 2);
    enc28j60_write_reg_16bit(E28J_ERXRDPT, next_frame_addr);
    enc28j60_set_reg_bits(E28J_ECON2, _BV(E28J_PKTDEC));
}

/**
 * enc28j60_alloc_packet - Allocates a new packet
 * @packet: A structure to receive the allocated packet buffer
 * @head_room: The space allocated for headers
 */
void enc28j60_alloc_packet(PacketBuf *packet, int head_room)
{
    packet->start = CFG_E28J_TRAN_BUFFER_START + head_room + 1;
    packet->length = 0;
}

/**
 * enc28j60_transmit - Transmits a packet
 * @packet: A packet buffer describing the packet to be sent
 * 
 * Note: the packet must have been initialized with all the appropriate
 * headers (including Ethernet), otherwise the results are undefined.
 * The function blocks until transmission is complete.
 */
void enc28j60_transmit(const PacketBuf *packet)
{
    uint16_t tries;
    uint8_t ctrl_byte = 0;
    
    enc28j60_write_mem_at(&ctrl_byte, packet->start - 1, 1); 
    
    enc28j60_write_reg_16bit(E28J_ETXST, packet->start - 1);
    enc28j60_write_reg_16bit(E28J_ETXND, packet->start + packet->length - 1);
    enc28j60_set_reg_bits(E28J_ECON1, _BV(E28J_TXRST));
    enc28j60_clear_reg_bits(E28J_ECON1, _BV(E28J_TXRST));
    enc28j60_set_reg_bits(E28J_ECON1, _BV(E28J_TXRTS));

    tries = 65535;
    while (enc28j60_read_reg(E28J_ECON1) & _BV(E28J_TXRTS)) {
        _delay_us(10);
        if (!tries--) {
            break;
        }
    }
}

/**
 * enc28j60_check_link_up - Checks that the Ethernet link is up.
 */
bool enc28j60_check_link_up(void)
{
    uint16_t stat = enc28j60_read_phy_reg(E28J_PHSTAT2);
    
    return ((stat >> E28J_LSTAT) & 1);
}

/**
 * enc28j60_init - Initialize ENC28J60 controller
 * @mac_addr: The initial MAC address
 */
uint8_t enc28j60_init(mac_addr_t* mac_addr)
{
    uint8_t rev_id;
    
    CFG_E28J_DDR |= _BV(CFG_E28J_CS_BIT) | _BV(CFG_E28J_RESET_BIT);

    enc28j60_hard_reset();

    rev_id = enc28j60_get_revision_id();
    if (rev_id == 0xFF) {
        return E_DEVICE;
    }

    // Organize memory
    enc28j60_write_reg_16bit(E28J_ERXST, CFG_E28J_RECV_BUFFER_START);
    enc28j60_write_reg_16bit(E28J_ERXRDPT, CFG_E28J_RECV_BUFFER_START);
    enc28j60_write_reg_16bit(E28J_ERXWRPT, CFG_E28J_RECV_BUFFER_START);
    enc28j60_write_reg_16bit(E28J_ERXND, CFG_E28J_RECV_BUFFER_END);
    enc28j60_write_reg_16bit(E28J_ETXST, CFG_E28J_TRAN_BUFFER_START);

    // Configure packet filter: accept unicast and broadcast packets
    // with valid CRC only
    enc28j60_write_reg(E28J_ERXFCON, _BV(E28J_UCEN) | _BV(E28J_CRCEN) |
        _BV(E28J_BCEN));

    // Configure MAC subsystem: flow control active, auto-padding to
    // 64 bytes, CRC and length check enabled
    enc28j60_write_reg(E28J_MACON2, 0);
    enc28j60_write_reg(E28J_MACON1, _BV(E28J_MARXEN));
    enc28j60_write_reg(E28J_MACON3, _BV(E28J_PADCFG1) | _BV(E28J_PADCFG0) |
        _BV(E28J_TXCRCEN) | _BV(E28J_FRMLNEN) | _BV(E28J_FULDPX));
    enc28j60_write_reg_16bit(E28J_MAMXFL, CFG_E28J_MAX_FRAME_SIZE);
    enc28j60_write_reg(E28J_MABBIPG, CFG_E28J_B2B_PACKET_GAP);
    enc28j60_write_reg_16bit(E28J_MAIPG, CFG_E28J_INTER_PACKET_GAP);
    enc28j60_set_mac_addr(mac_addr);

    // Configure PHY subsystem: full duplex, no loopback
    enc28j60_write_phy_reg(E28J_PHCON1, _BV(E28J_PDPXMD));
    enc28j60_write_phy_reg(E28J_PHCON2, _BV(E28J_HDLDIS));
    
    // Activate receiver
    enc28j60_set_reg_bits(E28J_ECON2, _BV(E28J_AUTOINC));
    enc28j60_set_reg_bits(E28J_ECON1, _BV(E28J_RXEN));

    return E_OK;
}
