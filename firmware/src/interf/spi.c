/*
 * spi.c - SPI interface functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include <avr/io.h>

#include "dot_config.h"
#include "spi.h"

#if (CFG_SPI_CLOCK_DIV==2) || (CFG_SPI_CLOCK_DIV==8) || (CFG_SPI_CLOCK_DIV==32)
#define SPI_SPI2X_VALUE _BV(SPI2X)
#else
#define SPI_SPI2X_VALUE 0
#endif

#if (CFG_SPI_CLOCK_DIV==2) || (CFG_SPI_CLOCK_DIV==4)
#define SPI_SPR_VALUE 0
#elif (CFG_SPI_CLOCK_DIV==8) || (CFG_SPI_CLOCK_DIV==16)
#define SPI_SPR_VALUE _BV(SPR0)
#elif (CFG_SPI_CLOCK_DIV==32) || (CFG_SPI_CLOCK_DIV==64)
#define SPI_SPR_VALUE _BV(SPR1)
#else
#define SPI_SPR_VALUE (_BV(SPR1)|_BV(SPR0))
#endif

/**
 * spi_slow - Switches to the lowest speed for the SPI interface
 * 
 * This is necessary for devices like SD cards, that demand low-speed
 * comunication between they are fully initialized.
 */
void spi_slow(void)
{
    SPCR &= ~(_BV(SPE));
    SPCR |= _BV(SPR0)|_BV(SPR1);
    SPSR = 0;
    SPCR |= _BV(SPE);
    spi_write(0xFF);
}

/**
 * spi_restore - Restores normal speed for the SPI interface
 */
void spi_restore(void)
{
    SPCR &= ~(_BV(SPE)|_BV(SPR0)|_BV(SPR1));
    SPCR |= SPI_SPR_VALUE;
    SPSR = SPI_SPI2X_VALUE;
    SPCR |= _BV(SPE);
    spi_write(0xFF);
}

/**
 * spi_init_master - Initializes the SPI interface in master mode
 */
void spi_init_master(void)
{
    CFG_SPI_DDR |= _BV(CFG_SPI_MOSI_BIT) | _BV(CFG_SPI_SS_BIT) | _BV(CFG_SPI_SCK_BIT);
    CFG_SPI_DDR &= ~(_BV(CFG_SPI_MISO_BIT));
    SPCR = _BV(SPE) | _BV(MSTR) | SPI_SPR_VALUE;
    SPSR = SPI_SPI2X_VALUE;
}
