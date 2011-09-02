/*
 * i2c.c - I2C/TWI communication functions
 * 
 * (C) Copyright 2009-2011  Cristian Dinu <goc9000@gmail.com>
 * 
 * Licensed under the GPL-3.
 */

#include "dot_config.h"
#include "i2c.h"

#if CFG_I2C_BAUD==100000
#define I2C_TWBR_VAL 72
#define I2C_TWSR_VAL 0
#else
#error "Invalid I2C baud specification"
#endif

static bool _i2c_wait(void)
{
    uint16_t tries = I2C_MAX_TRIES;
    
    while (tries && !(TWCR & _BV(TWINT))) {
        tries--;
    }
    
    return !!tries;
}

bool _i2c_start(void)
{
    TWCR = _BV(TWINT) + _BV(TWSTA) + _BV(TWEN);
    
    return _i2c_wait() &&
        ((i2c_status() == I2C_STAT_START) ||
         (i2c_status() == I2C_STAT_RESTART));
}

/**
 * i2c_start_read - Begins a READ operation on a slave device
 * @slave_addr: The 7-bit slave device address
 */
bool i2c_start_read(uint8_t slave_addr)
{
    if (!_i2c_start()) {
        return FALSE;
    }
    
    TWDR = (slave_addr << 1) + 1;
    TWCR = _BV(TWINT) + _BV(TWEN);
    
    return _i2c_wait() && (i2c_status() == I2C_STAT_R_SL_ACK);
}

/**
 * i2c_start_write - Begins a WRITE operation on a slave device
 * @slave_addr: The 7-bit slave device address
 */
bool i2c_start_write(uint8_t slave_addr)
{
    if (!_i2c_start()) {
        return FALSE;
    }
    
    TWDR = (slave_addr << 1) + 0;
    TWCR = _BV(TWINT) + _BV(TWEN);
    
    return _i2c_wait() && (i2c_status() == I2C_STAT_W_SL_ACK);
}

/**
 * i2sc_write - Writes a data byte to a slave
 * @data: The data byte to write
 */
bool i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = _BV(TWINT) + _BV(TWEN);
    
    return _i2c_wait() && (i2c_status() == I2C_STAT_W_DATA_ACK);
}

/**
 * i2sc_read - Reads a data byte from a slave
 */
bool i2c_read(void)
{
    TWCR = _BV(TWINT) + _BV(TWEA) + _BV(TWEN);
    _i2c_wait();
    
    return TWDR;
}


/**
 * i2sc_read_last - Reads the last data byte from a slave
 */
bool i2c_read_last(void)
{
    TWCR = _BV(TWINT) + _BV(TWEN);
    _i2c_wait();
    
    return TWDR;
}

/**
 * i2c_stop - Relinquishes the I2C bus
 */
void i2c_stop(void)
{
    TWCR = _BV(TWINT) + _BV(TWSTO) + _BV(TWEN);
    _i2c_wait();
}

/**
 * i2c_init - Initializes the IIC/TWI interface
 */
void i2c_init(void)
{
    DDRC &= 0xFC;
    PORTC |= 0x03;
    
    TWBR = I2C_TWBR_VAL;
    TWSR = I2C_TWSR_VAL;
    
    TWCR &= ~_BV(TWIE);
    TWCR |= _BV(TWEN);
}
