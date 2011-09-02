/*
 * sd_card.h - SD/MMC card functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _SD_CARD_H
#define _SD_CARD_H

#include <inttypes.h>

// Card commands
#define CARD_CMD_GO_IDLE_STATE      0
#define CARD_CMD_SET_BLOCKLEN      16
#define CARD_CMD_READ_SINGLE_BLOCK 17
#define CARD_CMD_WRITE_BLOCK       24
#define CARD_CMD_APP_CMD           55
#define CARD_CMD_CRC_ON_OFF        59

// Extended ("application") commands
#define CARD_ACMD_SD_SEND_OP_COND (0x80 | 41)

// Low-level card status codes
#define CARD_STAT_BUSY        0x00
#define CARD_STAT_IDLE        0xFF
#define CARD_STAT_BLOCK_START 0xFE

// R1 type response bits
#define CARD_R1_FLAG_IDLE        0x01
#define CARD_R1_FLAG_ERASE_RESET 0x02
#define CARD_R1_FLAG_BAD_COMMAND 0x04
#define CARD_R1_FLAG_CRC_ERROR   0x08
#define CARD_R1_FLAG_ERASE_ERROR 0x10
#define CARD_R1_FLAG_ADDR_ERROR  0x20
#define CARD_R1_FLAG_PARAM_ERROR 0x40
#define CARD_R1_FLAG_TIMEOUT     0x80

// Low-level functions
uint8_t card_exec(uint8_t command, uint32_t argument);
uint8_t card_response_size(uint8_t command);

// High-level functinos
uint8_t card_init(void);
uint8_t card_set_block_size(uint16_t blk_size);
uint8_t card_read_block(void *buffer, uint32_t address);
uint8_t card_write_block(const void *buffer, uint32_t address);

#endif
