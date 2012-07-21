/*
 * sd_card.c - SD/MMC card functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/io.h>
#include <util/delay.h>

#include "interf/spi.h"
#include "dot_config.h"
#include "util/bool.h"
#include "sd_card.h"

static uint16_t card_blk_size;

/**
 * card_exec - Executes an SD card command
 * @command: The command code (a CARD_CMD_* constant)
 * @argument: The command argument
 * 
 * Returns the card's answer. 0xFF means timeout.
 * 
 * Note: This does NOT automatically select/deselect the card!
 */
uint8_t card_exec(uint8_t command, uint32_t argument)
{
    uint8_t data;
    uint8_t *arg = (uint8_t *)(&argument);
    uint16_t tries;

    if (command & 0x80) {
        card_exec(CARD_CMD_APP_CMD, 0);
    }

    // Send command and argument
    spi_write(((command & 0x7F) | 0x40));
    spi_write(arg[3]);
    spi_write(arg[2]);
    spi_write(arg[1]);
    spi_write(arg[0]);
    spi_write(0x95); // Precomputed CRC for the initialization command

    // Receive answer
    for (tries = 1000; tries > 0; tries--) {
        spi_read(data);
        if ((data & 128) == 0) {
            break;
        }
    }

    return data;
}

/**
 * card_set_block_size - Sets the read/write block size
 * @blk_size: The block size
 * 
 * Note that the block size must be a power of two. Also, not many
 * cards support writing non-512-byte blocks.
 */
uint8_t card_set_block_size(uint16_t blk_size)
{
    uint8_t stat;
    
    card_blk_size = blk_size;
    
    card_chip_select();
    stat = card_exec(CARD_CMD_SET_BLOCKLEN, blk_size);
    card_chip_deselect();
    
    return stat;
}

/**
 * card_read_block - Reads a data block from the card
 * @buffer: A buffer to hold the data
 * @address: The block's starting address on the card
 */
uint8_t card_read_block(void *buffer, uint32_t address)
{
    uint8_t stat;
    uint16_t tries, i;

    card_chip_select();
    while (TRUE) {
        stat = card_exec(CARD_CMD_READ_SINGLE_BLOCK, address);
        if (stat) {
            break;
        }

        // Wait for block to be selected
        for (tries = 50000; tries > 0; tries--) {
            spi_read(stat);
            if (stat != CARD_STAT_IDLE) {
                break;
            }
        }
        if (!tries || (stat != CARD_STAT_BLOCK_START)) {
            break;
        }

        // Read block data
        for (i = 0; i < card_blk_size; i++) {
            spi_read(((uint8_t *)buffer)[i]);
        }

        // Ignores CRC word and idles for 8 more clocks
        spi_read(stat);
        spi_read(stat);
        spi_read(stat);

        stat = 0;
        break;
    }
    card_chip_deselect();

    return stat;
}

/**
 * card_writes_block - Writes a data block to the card
 * @buffer: A buffer holding the data to be written
 * @address: The block's starting address on the card
 */
uint8_t card_write_block(const void *buffer, uint32_t address)
{
    uint16_t i;
    uint16_t tries;
    uint8_t stat, dummy;
    
    card_chip_select();
    
    while (TRUE) {
        stat = card_exec(CARD_CMD_WRITE_BLOCK, address);
        if (stat) {
            break;
        }

        // Write block start
        spi_write(CARD_STAT_BLOCK_START);
        
        // Write block data
        for (i=0; i<card_blk_size; i++) {
            spi_write(((uint8_t *)buffer)[i]);
        }
        
        // Write dummy CRC
        spi_write(0xFF);
        spi_write(0xFF);

        // Read card feedback
        spi_read(stat);
        if ((stat & 0x1F) != 0x05) {
            break; // write error?
        }

        // Wait for write to complete
        for (tries = 50000; tries > 0; tries--)
        {
            spi_read(stat);
            if (stat != CARD_STAT_BUSY) {
                break;
            }
        }
        
        stat = tries ? 0 : 0xFF;
        break;
    }
    
    card_chip_deselect();
    spi_read(dummy);
    spi_read(dummy);

    return stat;
}

/**
 * card_init - Initializes the SD card
 *
 * Returns 0 if successful, otherwise an SD card error code
 */
uint8_t card_init(void)
{
    uint16_t i;
    uint16_t tries;
    uint8_t data;

    // Initializes chip select line
    CFG_CARD_CS_DDR |= _BV(CFG_CARD_CS_BIT);
    card_chip_deselect();

    // Transmit reset command.
    spi_slow();
    card_chip_select();
    for (tries = 10; tries > 0; tries--) {
        data = card_exec(CARD_CMD_GO_IDLE_STATE, 0);
        if (data == 0x00) {
            break;
        }
        for (i = 0; i < 100; i++) {
            spi_write(0xFF);
        }
    }
    card_chip_deselect();
    spi_restore();
    if (!tries) {
        return data;
    }

    // Transmit initialization command
    card_chip_select();
    for (tries = 20; tries > 0; tries--) {
        data = card_exec(CARD_ACMD_SD_SEND_OP_COND, 0);
        if (data == 0) {
            break;
        }
        for (i = 0; i < 100; i++) {
            spi_write(0xFF);
        }
    }
    card_chip_deselect();
    if (!tries) {
        return data;
    }

    return card_set_block_size(512);
}
