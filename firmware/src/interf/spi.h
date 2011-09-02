/*
 * spi.h - SPI interface functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _SPI_H
#define _SPI_H

/**
 * spi_wait - Waits for the current SPI operation to complete
 */
#define spi_wait() while (!(SPSR & _BV(SPIF)))

/**
 * spi_write - Sends a byte via SPI.
 * @data: The byte to be sent
 * 
 * The function blocks till completion. Make sure the SPI interface is
 * in the idle state at the time the function is called.
 */
#define spi_write(data) { SPDR = (data); spi_wait(); }

/**
 * spi_read - Reads a byte via SPI.
 * @data: The variable that will store the result
 * 
 * The function blocks till completion. Make sure the SPI interface is
 * in the idle state at the time the function is called.
 */
#define spi_read(var) { SPDR = 0xFF; spi_wait(); var = SPDR; }

void spi_init_master(void);
void spi_slow(void);
void spi_restore(void);

#endif
