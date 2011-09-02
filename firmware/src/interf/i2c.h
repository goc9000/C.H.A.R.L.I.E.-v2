/*
 * i2c.h - I2C/TWI interface functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#ifndef _I2C_H
#define _I2C_H

#include <avr/io.h>

#include "util/bool.h"

#define I2C_MAX_TRIES        2000

#define I2C_STAT_MASK        0xF8
#define I2C_STAT_ERROR       0x00
#define I2C_STAT_START       0x08
#define I2C_STAT_RESTART     0x10
#define I2C_STAT_W_SL_ACK    0x18
#define I2C_STAT_W_SL_NACK   0x20
#define I2C_STAT_W_DATA_ACK  0x28
#define I2C_STAT_W_DATA_NACK 0x30
#define I2C_STAT_ARB_LOST    0x38
#define I2C_STAT_R_SL_ACK    0x40
#define I2C_STAT_R_SL_NACK   0x48
#define I2C_STAT_R_DATA_ACK  0x50
#define I2C_STAT_R_DATA_NACK 0x58
#define I2C_STAT_IDLE        0xF8

#define i2c_status() (TWSR & I2C_STAT_MASK)

void i2c_init(void);
bool i2c_start_read(uint8_t slave_addr);
bool i2c_start_write(uint8_t slave_addr);
bool i2c_write(uint8_t data);
uint8_t i2c_read(void);
uint8_t i2c_read_last(void);
void i2c_stop(void);

#endif
